#pragma once

#include <juce_audio_processors/juce_audio_processors.h>


class GenericParameterListener final : public juce::AudioProcessorParameter::Listener
{
public:
  GenericParameterListener(const std::function<void()> clb) : callback(clb) {}
  ~GenericParameterListener() override {}

  void parameterValueChanged(int, float) override { callback(); }
  void parameterGestureChanged(int, bool) override {}

private:
  const std::function<void()> callback;
};
