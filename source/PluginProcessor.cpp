
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
        ),
        mainProcessor(new juce::AudioProcessorGraph()),
        muteInput(new juce::AudioParameterBool({ "mute", 1 }, "Mute input", false))
{
    addParameter(muteInput);
    
    audioInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::audioOutputNode));
    midiInputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiInputNode));
    midiOutputNode = mainProcessor->addNode(std::make_unique<AudioGraphIOProcessor>(AudioGraphIOProcessor::midiOutputNode));

    processorNodes.push_back(mainProcessor->addNode(std::make_unique<SamplerProcessor>()));
    processorNodes.push_back(mainProcessor->addNode(std::make_unique<GainProcessor>()));
    processorNodes.push_back(mainProcessor->addNode(std::make_unique<LevelProcessor>()));
}

#pragma mark -

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
    
    // this is a safety valve to protect us from too loud output
    // it kicks in when there appears to be some garbage in the output buffer (NaN, inf, or amp > 2)
    #if JUCE_DEBUG
        for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
            for (int i = 0; i < audioBuffer.getNumSamples(); ++i)
                jassert(!std::isinf(audioBuffer.getSample(ch, i)) && std::abs(audioBuffer.getSample(ch, i)) < 2.0f);
    #endif
}

#pragma mark -

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor(*this);
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

void PluginProcessor::updateGraph()
{
    //TODO: here we can update connections between audio processors
    
    for (auto node : processorNodes)
        node->setBypassed(static_cast<ProcessorBase*>(node->getProcessor())->bypass->get());

    audioInputNode->setBypassed(muteInput->get());
}

void PluginProcessor::connectAudioNodes()
{
    for (int ch = 0; ch < mainProcessor->getMainBusNumInputChannels(); ++ch)
    {
        switch (processorNodes.size()) {
            case 0:
                mainProcessor->addConnection({ { audioInputNode->nodeID, ch }, { audioOutputNode->nodeID, ch } });
                break;
            case 1:
                mainProcessor->addConnection({ { audioInputNode->nodeID, ch }, { processorNodes.front()->nodeID, ch } });
                mainProcessor->addConnection({ { processorNodes.front()->nodeID, ch }, { audioOutputNode->nodeID, ch } });
                break;
                
            default:
                Node::Ptr previousNode = audioInputNode;
                for (auto node : processorNodes)
                {
                    mainProcessor->addConnection({ { previousNode->nodeID, ch }, { node->nodeID, ch } });
                    previousNode = node;
                }
                mainProcessor->addConnection({ { previousNode->nodeID, ch }, { audioOutputNode->nodeID, ch } });
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
