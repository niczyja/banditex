
#include "processors/SamplerProcessor.h"
#include "gui/SamplerEditor.h"
#include <algorithm>
#include <iterator>
#include <random>


SamplerProcessor::SamplerProcessor()
{
    formatManager.registerBasicFormats();
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
    return new SamplerEditor(*this);
}

const juce::String SamplerProcessor::getName() const
{
    return "Sampler";
}

#pragma mark -

void SamplerProcessor::readFiles(juce::Array<juce::File>& files)
{
    reset();
    
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
        for (int ch = 0; ch < (int) reader->numChannels; ++ch)
            resampler.process(speedRatio, readBuffer.getReadPointer(ch), resampledBuffer.getWritePointer(ch), resampledBuffer.getNumSamples());
        
        SampleSpec sample = { i, samplesBuffer.getNumSamples(), samplesBuffer.getNumSamples() + resampledBuffer.getNumSamples() };
        samplesBuffer.setSize(getTotalNumOutputChannels(), sample.end, true);
        
        for (int ch = 0; ch < samplesBuffer.getNumChannels(); ++ch)
            samplesBuffer.copyFrom(ch, sample.start, resampledBuffer, ch % resampledBuffer.getNumChannels(), 0, resampledBuffer.getNumSamples());
        
        samplesSpecs.push_back(sample);
    }
    
    setIsShuffling(isShuffling);
}

int SamplerProcessor::getCurrentSampleIndex()
{
    if (currentSampleIndex == -1)
        return currentSampleIndex;
    
    return samplesSpecs[(size_t) currentSampleIndex].ordinal;
}

bool SamplerProcessor::getIsShuffling()
{
    return isShuffling;
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
    
    isShuffling = shouldShuffle;
    suspendProcessing(wasSuspended);
}

void SamplerProcessor::advanceToNextSample()
{
    ++currentSampleIndex;

    if (currentSampleIndex >= (int) samplesSpecs.size())
    {
        if (isShuffling)
            std::shuffle(samplesSpecs.begin(), samplesSpecs.end(), std::mt19937());
        
        currentSampleIndex = (isLooping ? 0 : -1);
    }
    
    currentPosition = (currentSampleIndex == -1 ? 0 : samplesSpecs[(size_t) currentSampleIndex].start);

    sendChangeMessage();
}
