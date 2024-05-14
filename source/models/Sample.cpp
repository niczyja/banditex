
#include "Sample.h"


Sample::Sample(juce::AudioFormatReader& reader, double maxLengthSeconds, double destSampleRate) :
    sampleRate(reader.sampleRate),
    numSamples(juce::jmin(int(reader.lengthInSamples), int(maxLengthSeconds * sampleRate))),
    tempBuffer(int(reader.numChannels), numSamples)
{
    if (numSamples == 0)
        throw std::runtime_error("Invalid sample length.");
    
    reader.read(&tempBuffer, 0, numSamples, 0, true, true);
    resample(sampleRate / destSampleRate);
}

double Sample::getSampleRate() const
{
    return sampleRate;
}

int Sample::getNumSamples() const
{
    return numSamples;
}

const juce::AudioSampleBuffer& Sample::getBuffer() const
{
    return dataBuffer;
}

void Sample::applyGain(const float gain)
{
    dataBuffer.applyGain(gain);
}

void Sample::resample(double sampleRatio)
{
    int newNumSamples = (int) (tempBuffer.getNumSamples() / sampleRatio);
    dataBuffer.setSize(tempBuffer.getNumChannels(), newNumSamples);

    juce::LagrangeInterpolator resampler;
    for (int ch = 0; ch < dataBuffer.getNumChannels(); ++ch)
        resampler.process(sampleRatio, tempBuffer.getReadPointer(ch), dataBuffer.getWritePointer(ch), dataBuffer.getNumSamples());
    
    sampleRate /= sampleRatio;
    numSamples = newNumSamples;
    
    tempBuffer.clear();
}
