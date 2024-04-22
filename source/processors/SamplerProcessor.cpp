
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
    fileBuffers.clear();
    bufferOrder.clear();
}

void SamplerProcessor::processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer&)
{
    if (currentBufferIndex == -1)
        advanceBufferIndex();

    auto fileBuffer = fileBuffers[(size_t)getCurrentFileIndex()];
    auto numOutputChannels = audioBuffer.getNumChannels();
    auto outSamplesRemaining = audioBuffer.getNumSamples();
    auto outSamplesOffset = 0;
    
    while (outSamplesRemaining > 0)
    {
        auto bufSamplesRemaining = fileBuffer.getNumSamples() - position;
        auto samplesThisTime = juce::jmin(outSamplesRemaining, bufSamplesRemaining);
        
        for (auto ch = 0; ch < numOutputChannels; ++ch)
            audioBuffer.copyFrom(ch, outSamplesOffset, fileBuffer, ch % fileBuffer.getNumChannels(), position, samplesThisTime);
        
        outSamplesRemaining -= samplesThisTime;
        outSamplesOffset += samplesThisTime;
        position += samplesThisTime;
        
        if (position == fileBuffer.getNumSamples())
        {
            advanceBufferIndex();
            if (currentBufferIndex == -1)
            {
                suspendProcessing(true);
                sendChangeMessage();
            }
            position = 0;
        }
    }
}

void SamplerProcessor::reset()
{
    suspendProcessing(true);
    releaseResources();
    currentBufferIndex = -1;
    position = 0;

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
    
    for (auto file : files)
    {
        std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor(file));
        if (reader.get() == nullptr)
            continue;
        
        juce::AudioSampleBuffer fileBuffer;
        fileBuffer.setSize((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read(&fileBuffer, 0, (int) reader->lengthInSamples, 0, true, true);
        fileBuffers.push_back(fileBuffer);
    }
    
    setIsShuffling(isShuffling);
}

int SamplerProcessor::getCurrentFileIndex()
{
    return bufferOrder[(size_t) currentBufferIndex];
}

bool SamplerProcessor::getIsShuffling()
{
    return isShuffling;
}

void SamplerProcessor::setIsShuffling(bool shouldShuffle)
{
    bufferOrder.clear();
    
    for (int i = 0; i < (int) fileBuffers.size(); ++i)
        bufferOrder.push_back(i);
    
    if (shouldShuffle)
        std::shuffle(bufferOrder.begin(), bufferOrder.end(), std::mt19937());
    
    isShuffling = shouldShuffle;
}

void SamplerProcessor::advanceBufferIndex()
{
    ++currentBufferIndex;
    if (currentBufferIndex >= (int) fileBuffers.size())
        currentBufferIndex = (isLooping ? 0 : -1);

    sendChangeMessage();
}
