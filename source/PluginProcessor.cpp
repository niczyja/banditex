
#include "PluginProcessor.h"
#include "PluginEditor.h"


PluginProcessor::PluginProcessor() 
    : AudioProcessor
        (BusesProperties()
            #if ! JucePlugin_IsMidiEffect
            #if ! JucePlugin_IsSynth
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
            #endif
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
            #endif
        ),
        mainProcessor(new juce::AudioProcessorGraph()),
        muteInput(new juce::AudioParameterBool({ "mute", 1 }, "Mute input", false))
{
    addParameter(muteInput);
    
    //TODO: this needs to go to initializeGraph() as soon as we figure out clean connection with UI
    processorNode = mainProcessor->addNode(std::make_unique<TestPlaygroundProcessor>());
}

#pragma mark -

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    #if JucePlugin_IsMidiEffect
        juce::ignoreUnused (layouts);
        return true;
    #else
        if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
            return false;

        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;
    
        #if ! JucePlugin_IsSynth
            if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
                return false;
        #endif
    
        return true;
    #endif
}

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mainProcessor->setPlayConfigDetails(getMainBusNumInputChannels(),
                                        getMainBusNumOutputChannels(),
                                        sampleRate, samplesPerBlock);
    
    mainProcessor->prepareToPlay(sampleRate, samplesPerBlock);
    initializeGraph();
}

void PluginProcessor::releaseResources()
{
    mainProcessor->releaseResources();
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer)
{
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        audioBuffer.clear(i, 0, audioBuffer.getNumSamples());
    
    updateGraph();
    mainProcessor->processBlock(audioBuffer, midiBuffer);
}

#pragma mark -

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    //TODO: replace with proper component composition (PluginEditor should layout UI components of other processors)
    TestPlaygroundProcessor *playgroundProcessor = static_cast<TestPlaygroundProcessor*>(processorNode->getProcessor());
    return new PluginEditor(*playgroundProcessor);
}

bool PluginProcessor::hasEditor() const
{
    return true;
}

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

#pragma mark -

void PluginProcessor::initializeGraph()
{
    //TODO: this needs to come back as soon as we figure out clean connection with UI
    //    mainProcessor->clear();
    
    audioInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
    midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
    midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));
    
    processorNode->getProcessor()->setPlayConfigDetails(getMainBusNumInputChannels(), getMainBusNumOutputChannels(),
                                                        getSampleRate(), getBlockSize());

    for (int ch = 0; ch < 2; ++ch) {
        mainProcessor->addConnection({ { audioInputNode->nodeID, ch }, { processorNode->nodeID, ch } });
        mainProcessor->addConnection({ { processorNode->nodeID, ch }, { audioOutputNode->nodeID, ch } });
    }

    connectMidiNodes();

    for (auto node: mainProcessor->getNodes())
        node->getProcessor()->enableAllBuses();
}

void PluginProcessor::updateGraph()
{
    //TODO: here we can update connections between audio processors
    
    audioInputNode->setBypassed(muteInput->get());
}

void PluginProcessor::connectAudioNodes()
{
    // default connection bypassing all processors, just input connected to output
    
    for (int ch = 0; ch < 2; ++ch)
        mainProcessor->addConnection({ { audioInputNode->nodeID, ch }, { audioOutputNode->nodeID, ch } });
}

void PluginProcessor::connectMidiNodes()
{
    auto ch = juce::AudioProcessorGraph::midiChannelIndex;
    mainProcessor->addConnection({ { midiInputNode->nodeID, ch }, { midiOutputNode->nodeID, ch } });
}

#pragma mark -

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
