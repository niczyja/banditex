#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    
    mFormatManager.registerBasicFormats();
    
    //adding voices for audio playback
    for (int i = 0; i < mNumVoices; i++)
    {
        mSampler.addVoice(new juce::SamplerVoice());
    }
    
}

PluginProcessor::~PluginProcessor()
{
    mFormatReader = nullptr;
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //making sure that mSampler uses a correct sample rate defined by DAW
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}




//---------------------------------------------------------------
//----------------- PROCESS BLOCK -------------------------------
//---------------------------------------------------------------




void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Render next block
    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Iterate over incoming MIDI messages
    for (const auto metadata : midiMessages)
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
    midiMessages.clear();

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
        // ..do something to the data...
    }
}



//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}




//---------------------------------------------------------------
//---------------------BANDITEX METHODS--------------------------
//---------------------------------------------------------------



void PluginProcessor::loadFiles()
{
    // Method for loading audio files through an openable window
    juce::FileChooser chooser {"Load Files"};
    if (chooser.browseForMultipleFilesToOpen())
    {
        clearFiles(); // Clear previously loaded files if any
        
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

            // Call the updateLoadedFilesListAndHighlight() function of the editor to update the UI
            if (auto* editor = dynamic_cast<PluginEditor*>(getActiveEditor()))
            {
                editor->updateLoadedFilesList();
            }
        }
    }
}

void PluginProcessor::setCurrentlyPlayingFileIndex(int newIndex)
{
    if (currentlyPlayingFileIndex == newIndex) {
        return;
    }
    
    currentlyPlayingFileIndex = newIndex;
    sendChangeMessage(); // Notify all registered listeners about the change
}

int PluginProcessor::getCurrentlyPlayingFileIndex() const
{
    return currentlyPlayingFileIndex;
}

void PluginProcessor::clearFiles()
{
    mSampler.clearSounds();
    loadedFiles.clear();
}

juce::StringArray PluginProcessor::getLoadedFilesNames()
{
    juce::StringArray loadedFilesNames;
    for (const auto& file : loadedFiles)
    {
        loadedFilesNames.add(file.getFileName());
    }
    return loadedFilesNames;
}

std::optional<int> PluginProcessor::getRandomFileIndex() const
{
    // Get the number of files in loadedFiles to pick a random index
    int numFiles = static_cast<int>(loadedFiles.size());
    
    if (numFiles == 0) {
        return std::nullopt;
    }
    
    return juce::Random::getSystemRandom().nextInt(numFiles);
}

std::optional<juce::File> PluginProcessor::getFileAtIndex (int index) const
{
    int numFiles = static_cast<int>(loadedFiles.size());
    
    if (index < 0 || index > numFiles - 1) {
        return std::nullopt;
    }
    
    return loadedFiles[static_cast<std::vector<juce::File>::size_type>(index)];
}

//==============================================================================
// This creates new instances of the plugin..


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
