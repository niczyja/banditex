#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginParameters.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor final : public juce::AudioProcessor
{
public:
    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
    using NodeAndChannel = juce::AudioProcessorGraph::NodeAndChannel;
    using Node = juce::AudioProcessorGraph::Node;
    
    PluginProcessor();
    ~PluginProcessor() override;

    const juce::String getName() const override;

    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;

    bool isMidiEffect() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    double getTailLengthSeconds() const override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override;

    //TODO: figure out how to make it private:
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
private:
    std::unique_ptr<PluginParameters> parameters;
    
    Node::Ptr audioOutputNode;
    Node::Ptr midiInputNode;
    Node::Ptr midiOutputNode;
    std::vector<Node::Ptr> processorNodes;
    
    void connectAudioNodes();
    void connectMidiNodes();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
