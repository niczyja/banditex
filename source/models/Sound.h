#pragma once

#include "Sample.h"


class Sound final
{
public:
    using PlaybackRange = juce::Range<double>;
    
    void setSample(std::unique_ptr<Sample> value);
    Sample* getSample() const;
    
    void setPlaybackRange(const PlaybackRange range);
    PlaybackRange getPlaybackRange() const;
    
    void setBypass(bool isBypassed);
    bool getBypass() const;
    
    void setGain(float newGain);
    float getGain() const;
    
private:
    std::unique_ptr<Sample> sample;
    PlaybackRange playbackRange;
    bool bypass = false;
    float gain = 1.0f;
};
