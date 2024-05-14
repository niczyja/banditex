#pragma once

#include <juce_audio_formats/juce_audio_formats.h>


class Sample final
{
public:
    Sample(juce::AudioFormatReader& reader, double maxLengthSeconds, double destSampleRate);
    
    double getSampleRate() const;
    int getNumSamples() const;
    const juce::AudioSampleBuffer& getBuffer() const;
    void applyGain(const float gain);
    
private:
    double sampleRate;
    int numSamples;
    juce::AudioSampleBuffer tempBuffer;
    juce::AudioSampleBuffer dataBuffer;
    
    void resample(double sampleRatio);
};
