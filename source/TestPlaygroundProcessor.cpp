
#include "TestPlaygroundProcessor.h"
#include "PluginEditor.h"


TestPlaygroundProcessor::TestPlaygroundProcessor()
{
    mFormatManager.registerBasicFormats();
             
    //adding parameter values for pitch shift, offset and randomisation
    //    addParameter(pitchOffset = new juce::AudioParameterFloat("pitchOffset", "Pitch Offset", -24.0f, 24.0f, 0.0f));
    //    addParameter(pitchWheel = new juce::AudioParameterFloat("pitchWheel", "Pitch Wheel", -12.0f, 12.0f, 0.0f));
    //    addParameter(randomPitchRange = new juce::AudioParameterFloat("randomPitchRange", "Random Pitch Range", -12.0f, 12.0f, 0.0f));
    
    //adding voices for audio playback
    for (int i = 0; i < mNumVoices; i++)
    {
        mSampler.addVoice(new juce::SamplerVoice());
    }
}

#pragma mark - PREPARE & PROCESS

void TestPlaygroundProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    //making sure that mSampler uses a correct sample rate defined by DAW
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
    
    juce::ignoreUnused (samplesPerBlock);
}

void TestPlaygroundProcessor::releaseResources()
{
    
}

void TestPlaygroundProcessor::processBlock(juce::AudioBuffer<float> & audioBuffer, juce::MidiBuffer & midiBuffer)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        audioBuffer.clear(i, 0, audioBuffer.getNumSamples());
    
    // Accessing the parameter values of pitch wheel, pitch offset and pitch randomisation
    // auto pitchValue = *pitchOffset + *pitchWheel + juce::Random::getSystemRandom().nextFloat() * *randomPitchRange;

    // Render next block
    mSampler.renderNextBlock(audioBuffer, midiBuffer, 0, audioBuffer.getNumSamples());

    // Iterate over incoming MIDI messages
    for (const auto metadata : midiBuffer)
    {
        const juce::MidiMessage& midiEvent = metadata.getMessage();
        
        if (midiEvent.isNoteOn())
        {
            if (auto randomIndex = getRandomFileIndex())
            {
                mSampler.noteOn(midiEvent.getChannel(), *randomIndex, midiEvent.getFloatVelocity());
                setCurrentlyPlayingFileIndex(*randomIndex);
            }
        }
        else if (midiEvent.isNoteOff())
        {
            mSampler.allNotesOff(midiEvent.getChannel(), true);
            setCurrentlyPlayingFileIndex(-1);
        }
    }

    // Remove processed MIDI messages
    midiBuffer.clear();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = audioBuffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
        // ..do something to the data...
    }
}

#pragma mark - BANDITEX PLAYGROUND

void TestPlaygroundProcessor::loadFiles()
{
    clearFiles();

    fileChooser = std::make_unique<juce::FileChooser>("Load files...");
    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems;
    
    fileChooser->launchAsync(flags, [this] (const juce::FileChooser &chooser)
    {
        auto files = chooser.getResults();
        int index = 0;

        for (auto file : files)
        {
            DBG("Loaded file: " << file.getFullPathName());
            
            auto audioFileReader = mFormatManager.createReaderFor(file);
            if (audioFileReader != nullptr)
            {
                // Set MIDI note on which sound will be played
                juce::BigInteger range;
                range.setRange(index, 1, true);
                
                // Create sound and add to sampler
                auto newSound = new juce::SamplerSound (file.getFileName(), *audioFileReader, range, 0, 0.1, 0.1, 30);
                mSampler.addSound(newSound);

                // Store the loaded file
                loadedFiles.push_back(file);
                
                index++;
            }
            else
            {
                DBG("Error loading file: " << file.getFullPathName());
            }

            // Broadcast changes to update the UI
            sendChangeMessage();
        }
    });
}

void TestPlaygroundProcessor::setCurrentlyPlayingFileIndex(int newIndex)
{
    if (currentlyPlayingFileIndex == newIndex) {
        return;
    }
    
    currentlyPlayingFileIndex = newIndex;
    sendChangeMessage(); // Notify all registered listeners about the change
}

int TestPlaygroundProcessor::getCurrentlyPlayingFileIndex() const
{
    return currentlyPlayingFileIndex;
}

void TestPlaygroundProcessor::clearFiles()
{
    mSampler.clearSounds();
    loadedFiles.clear();
}

juce::StringArray TestPlaygroundProcessor::getLoadedFilesNames()
{
    juce::StringArray loadedFilesNames;
    for (const auto& file : loadedFiles)
    {
        loadedFilesNames.add(file.getFileName());
    }
    return loadedFilesNames;
}

std::optional<int> TestPlaygroundProcessor::getRandomFileIndex() const
{
    // Get the number of files in loadedFiles to pick a random index
    int numFiles = static_cast<int>(loadedFiles.size());
    
    if (numFiles == 0) {
        return std::nullopt;
    }
    
    return juce::Random::getSystemRandom().nextInt(numFiles);
}

std::optional<juce::File> TestPlaygroundProcessor::getFileAtIndex (int index) const
{
    int numFiles = static_cast<int>(loadedFiles.size());
    
    if (index < 0 || index > numFiles - 1) {
        return std::nullopt;
    }
    
    return loadedFiles[static_cast<std::vector<juce::File>::size_type>(index)];
}

