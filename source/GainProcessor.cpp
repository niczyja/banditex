
#include "GainProcessor.h"


GainProcessor::GainProcessor()
    : ProcessorBase(),
    gain(new juce::AudioParameterFloat({ "gain", 1 }, "Gain", 0.0f, 2.0f, 1.0f))
{
    addParameter(gain);
}

void GainProcessor::processBlock(juce::AudioBuffer<float> &audioBuffer, juce::MidiBuffer &)
{
    audioBuffer.applyGain(*gain);
}

void GainProcessor::reset()
{
    *gain = 1.0f;
}

const juce::String GainProcessor::getName() const
{
    return "Gain";
}
