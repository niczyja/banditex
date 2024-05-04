
#include "processors/SamplerProcessor.h"
#include "gui/SamplerEditor.h"
#include <algorithm>
#include <iterator>
#include <random>


SamplerProcessor::SamplerProcessor()
    : ProcessorBase(),
    parameters(*this, nullptr, juce::Identifier ("Sampler Parameters"), {
        std::make_unique<juce::AudioParameterBool> (juce::ParameterID ("bypass", 1), "Bypass", false),
        std::make_unique<juce::AudioParameterBool> (juce::ParameterID ("loop", 1), "Loop", false),
        std::make_unique<juce::AudioParameterBool> (juce::ParameterID ("shuffle", 1), "Shuffle", false),
        std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("pitch", 1), "Pitch", -2.5f, 2.5f, 0.0f),
        std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("level", 1), "Level", 0.0f, 1.0f, 0.75f)
    })
{
    formatManager.registerBasicFormats();
    parameters.addParameterListener("shuffle", this);
}

SamplerProcessor::~SamplerProcessor()
{
    parameters.removeParameterListener("shuffle", this);
}

void SamplerProcessor::prepareToPlay (double, int)
{
    reset();
}

void SamplerProcessor::releaseResources()
{
    samplesBuffer.setSize(0, 0);
    samplesSpecs.clear();
}

void SamplerProcessor::processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer&)
{
    if (getBypassParameter()->getValue() > 0.5f)
        return;
    
    if (currentSampleIndex == -1)
        advanceToNextSample();

    auto numOutputChannels = audioBuffer.getNumChannels();
    auto outSamplesRemaining = audioBuffer.getNumSamples();
    auto outSamplesOffset = 0;
    
    while (outSamplesRemaining > 0)
    {
        auto currentSampleSpec = samplesSpecs[(size_t) currentSampleIndex];
        auto inSamplesRemaining = currentSampleSpec.end - currentPosition;
        auto inSamplesThisTime = juce::jmin(outSamplesRemaining, inSamplesRemaining);
        
        for (auto ch = 0; ch < numOutputChannels; ++ch)
            audioBuffer.copyFrom(ch, outSamplesOffset, samplesBuffer, ch % samplesBuffer.getNumChannels(), currentPosition, inSamplesThisTime);
        
        outSamplesRemaining -= inSamplesThisTime;
        outSamplesOffset += inSamplesThisTime;
        currentPosition += inSamplesThisTime;

        if (currentPosition == currentSampleSpec.end)
        {
            advanceToNextSample();
            if (currentSampleIndex == -1)
            {
                suspendProcessing(true);
                sendChangeMessage();
                break;
            }
        }
    }
    
    for (int ch = 0; ch < audioBuffer.getNumChannels(); ++ch)
        for (int sample = 0; sample < audioBuffer.getNumSamples(); ++sample)
            audioBuffer.setSample(ch, sample, audioBuffer.getSample(ch, sample) * *parameters.getRawParameterValue("level"));
}

void SamplerProcessor::reset()
{
    suspendProcessing(true);
    releaseResources();
    currentSampleIndex = -1;
    currentPosition = 0;

    sendChangeMessage();
}

#pragma mark -

juce::AudioProcessorEditor* SamplerProcessor::createEditor()
{
    return new SamplerEditor(*this, this->parameters);
}

juce::AudioProcessorParameter* SamplerProcessor::getBypassParameter() const
{
    return parameters.getParameter("bypass");
}

const juce::String SamplerProcessor::getName() const
{
    return "Sampler";
}

#pragma mark -

void SamplerProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "shuffle")
        setIsShuffling(newValue > 0.5f);
}

#pragma mark -

void SamplerProcessor::readFiles(juce::Array<juce::File>& files)
{
    reset();
    
    const int chunkSize = 1024;
    
    for (int i = 0; i < files.size(); ++i)
    {
        auto file = files[i];
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
        if (reader.get() == nullptr)
            continue;
        
        double speedRatio = reader->sampleRate / getSampleRate();

        juce::AudioSampleBuffer readBuffer ((int) reader->numChannels, (int) reader->lengthInSamples);
        juce::AudioSampleBuffer resampledBuffer ((int) reader->numChannels, (int) (reader->lengthInSamples / speedRatio));

        reader->read(&readBuffer, 0, (int) reader->lengthInSamples, 0, true, true);
        juce::LagrangeInterpolator resampler;
        
        std::vector<float> singleWaveformPeak;
        for (int position = 0; position < readBuffer.getNumSamples(); position += chunkSize)
        {
            float minSampleValue = std::numeric_limits<float>::max();
            float maxSampleValue = std::numeric_limits<float>::lowest();
            int limit = juce::jmin(position + chunkSize, readBuffer.getNumSamples());
            
            for (int j = position; j < limit; ++j)
            {
                float sample = readBuffer.getSample(0, j);  // Only using the first channel
                if (sample < minSampleValue) minSampleValue = sample;
                if (sample > maxSampleValue) maxSampleValue = sample;
            }
            
            singleWaveformPeak.push_back(minSampleValue);
            singleWaveformPeak.push_back(maxSampleValue);
            
        }
        
        waveformPeaks.push_back(singleWaveformPeak);
        
        for (int ch = 0; ch < (int) reader->numChannels; ++ch)
            resampler.process(speedRatio, readBuffer.getReadPointer(ch), resampledBuffer.getWritePointer(ch), resampledBuffer.getNumSamples());
        
        SampleSpec sample = { i, samplesBuffer.getNumSamples(), samplesBuffer.getNumSamples() + resampledBuffer.getNumSamples() };
        samplesBuffer.setSize(getTotalNumOutputChannels(), sample.end, true);
        
        for (int ch = 0; ch < samplesBuffer.getNumChannels(); ++ch)
            samplesBuffer.copyFrom(ch, sample.start, resampledBuffer, ch % resampledBuffer.getNumChannels(), 0, resampledBuffer.getNumSamples());
        
        samplesSpecs.push_back(sample);
    }
    
    setIsShuffling(*parameters.getRawParameterValue("shuffle") > 0.5f);
}

int SamplerProcessor::getCurrentSampleIndex()
{
    if (currentSampleIndex == -1)
        return currentSampleIndex;
    
    return samplesSpecs[(size_t) currentSampleIndex].ordinal;
}

void SamplerProcessor::setIsShuffling(bool shouldShuffle)
{
    bool wasSuspended = isSuspended();
    suspendProcessing(true);
    
    currentSampleIndex = -1;
    currentPosition = 0;

    if (shouldShuffle)
        std::shuffle(samplesSpecs.begin(), samplesSpecs.end(), std::mt19937());
    else
        std::sort(samplesSpecs.begin(), samplesSpecs.end());
    
    suspendProcessing(wasSuspended);
}

void SamplerProcessor::advanceToNextSample()
{
    ++currentSampleIndex;

    if (currentSampleIndex >= (int) samplesSpecs.size())
    {
        if (*parameters.getRawParameterValue("shuffle") > 0.5f)
            std::shuffle(samplesSpecs.begin(), samplesSpecs.end(), std::mt19937());
        
        currentSampleIndex = (*parameters.getRawParameterValue("loop") > 0.5f ? 0 : -1);
    }
    
    currentPosition = (currentSampleIndex == -1 ? 0 : samplesSpecs[(size_t) currentSampleIndex].start);

    sendChangeMessage();
}
