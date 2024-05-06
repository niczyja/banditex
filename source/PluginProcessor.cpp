
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

#include "processors/SamplerProcessor.h"
#include "processors/LevelProcessor.h"
#include "processors/GainProcessor.h"


PluginProcessor::PluginProcessor() 
    : AudioProcessor
        (BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        )
{
    mainProcessor = std::make_unique<juce::AudioProcessorGraph>();
    parameters = std::make_unique<PluginParameters>(*this);
    
    audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
    midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
    midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));
    processorNodes.push_back(mainProcessor->addNode(std::make_unique<SamplerProcessor>()));
}

PluginProcessor::~PluginProcessor()
{
}

#pragma mark -

const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

#pragma mark -

bool PluginProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
}

#pragma mark -

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
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

#pragma mark -

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

juce::AudioProcessorParameter* PluginProcessor::getBypassParameter() const
{
    return parameters->raw("bypass");
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled())
        return false;
    
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

#pragma mark -

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

void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mainProcessor->setPlayConfigDetails(getMainBusNumInputChannels(),
                                        getMainBusNumOutputChannels(),
                                        sampleRate, samplesPerBlock);
    
    mainProcessor->prepareToPlay(sampleRate, samplesPerBlock);
    
    for (auto connection : mainProcessor->getConnections())
        mainProcessor->removeConnection(connection);
    
    for (auto node : mainProcessor->getNodes())
        node->getProcessor()->setPlayConfigDetails(mainProcessor->getMainBusNumInputChannels(),
                                                   mainProcessor->getMainBusNumOutputChannels(),
                                                   mainProcessor->getSampleRate(),
                                                   mainProcessor->getBlockSize());
    
    connectAudioNodes();
    connectMidiNodes();

    for (auto node: mainProcessor->getNodes())
        node->getProcessor()->enableAllBuses();
}

void PluginProcessor::releaseResources()
{
    mainProcessor->releaseResources();
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer)
{
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        audioBuffer.clear(i, 0, audioBuffer.getNumSamples());
    
    //TODO: here we can update connections between audio processors (adding/removing samplers)

    if (!parameters->bypass())
        mainProcessor->processBlock(audioBuffer, midiBuffer);
    
    // this is a safety valve to protect us from too loud output
    // it kicks in when there appears to be some garbage in the output buffer (NaN, inf, or amplitude > 2)
    #if JUCE_DEBUG
        for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
            for (int i = 0; i < audioBuffer.getNumSamples(); ++i)
                jassert(!std::isinf(audioBuffer.getSample(ch, i)) && std::abs(audioBuffer.getSample(ch, i)) < 2.0f);
    #endif
}

#pragma mark -

void PluginProcessor::connectAudioNodes()
{
    for (int ch = 0; ch < mainProcessor->getMainBusNumInputChannels(); ++ch)
    {
        switch (processorNodes.size()) {
            case 0:
                break;
            case 1:
                mainProcessor->addConnection({ { processorNodes.front()->nodeID, ch }, { audioOutputNode->nodeID, ch } });
                break;
            default:
                auto node = processorNodes.begin();
                do {
                    NodeAndChannel source = { node->get()->nodeID, ch };
                    std::advance(node, 1);
                    NodeAndChannel dest = { node->get()->nodeID, ch };
                    mainProcessor->addConnection({ source, dest });
                }
                while (node != processorNodes.end());
                
                mainProcessor->addConnection({ { node->get()->nodeID, ch }, { audioOutputNode->nodeID, ch } });
                break;
        }
    }
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
