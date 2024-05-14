
#include "Sound.h"


void Sound::setSample(std::unique_ptr<Sample> value)
{
    sample = std::move(value);
    setPlaybackRange(playbackRange);
    setGain(gain);
}

Sample* Sound::getSample() const
{
    return sample.get();
}

void Sound::setPlaybackRange(const PlaybackRange range)
{
    if (sample == nullptr)
        playbackRange = range;
    else
        playbackRange = PlaybackRange(0, sample->getNumSamples()).constrainRange(range);
}

Sound::PlaybackRange Sound::getPlaybackRange() const
{
    return playbackRange;
}

void Sound::setBypass(bool isBypassed)
{
    bypass = isBypassed;
}

bool Sound::getBypass() const
{
    return bypass;
}

void Sound::setGain(float newGain)
{
    if (sample != nullptr)
        sample->applyGain(newGain / gain);

    gain = newGain;
}

float Sound::getGain() const
{
    return gain;
}
