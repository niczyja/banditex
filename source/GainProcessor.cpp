
#include "GainProcessor.h"


GainProcessor::GainProcessor()
    : ProcessorBase(),
    gain(new juce::AudioParameterFloat("gain", "Gain", -6.0f, 6.0f, 1.0f))
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
