#pragma once

#include "parameters/GenericParameterContainer.h"
#include "SamplerUtils.h"


class SamplerParameters final
{
public:
    explicit SamplerParameters(juce::AudioProcessor& proc);
    ~SamplerParameters();

    void onBypass(std::function<void()> callback);
    void onLoopMode(std::function<void()> callback);
    void onPlaybackOrder(std::function<void()> callback);
    void onReset(std::function<void()> callback);

    juce::AudioProcessorParameter* raw(const std::string& id) const;
    
    bool bypass() const;
    LoopMode loopMode() const;
    float loopFadeLength() const;
    float loopTriggerRate() const;
    float loopGapLength() const;
    PlaybackOrder playbackOrder() const;
    
    void read(const void* data, const int size);
    void write(juce::MemoryBlock& data);

private:

    const int schema = 1;
    const int maxSounds = 50;
    const int maxSoundLength = 30;
    
    const float minFadeLength = 0.0f;
    const float maxFadeLength = 5.0f;
    
    const float minTriggerRate = 0.1f;
    const float maxTriggerRate = 5.0f;
    
    const float minGapLength = 0.0f;
    const float maxGapLength = 5.0f;
    
    GenericParameterContainer parameters;
};
