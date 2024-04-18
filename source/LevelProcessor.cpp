
#include "LevelProcessor.h"


LevelProcessor::LevelProcessor()
    : ProcessorBase(),
    level(new juce::AudioParameterFloat({ "level", 1 }, "Level", 0.0f, 2.0f, 1.0f))
{
    addParameter(level);
}

void LevelProcessor::processBlock(juce::AudioBuffer<float> &audioBuffer, juce::MidiBuffer &)
{
    for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
        for (int sample = 0; sample < audioBuffer.getNumSamples(); ++sample)
            audioBuffer.setSample(ch, sample, audioBuffer.getSample(ch, sample) * *level);
}

void LevelProcessor::reset()
{
    *level = 1.0f;
}

const juce::String LevelProcessor::getName() const
{
    return "Level";
}
