#pragma once

#include "parameters/GenericParameterContainer.h"


class PluginParameters final
{
public:
    explicit PluginParameters(juce::AudioProcessor& proc);
    ~PluginParameters();
    
    void onbypass(std::function<void()> callback);
    void onnumsamplers(std::function<void()> callback);
    void onreset(std::function<void()> callback);
    
    juce::AudioProcessorParameter* raw(const std::string& id) const;
    
    bool bypass() const;
    int numsamplers(const int newnumsamplers) const;
    
    void read(const void* data, const int size);
    void write(juce::MemoryBlock& data);
    
private:
    
    const int schema = 1;
    const int maxsamplers = 5;
    
    GenericParameterContainer parameters;
};
