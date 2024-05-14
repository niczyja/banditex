
#include "SamplerParameters.h"


SamplerParameters::SamplerParameters(juce::AudioProcessor& proc)
    : parameters(proc)
{
    parameters.add("bypass", new juce::AudioParameterBool({ "bypass", schema }, "Bypass", false, juce::AudioParameterBoolAttributes()));
    parameters.add("loop", new juce::AudioParameterChoice({ "loopmode", schema }, "Loop mode", juce::StringArray(LoopMode::labels), 0));
    parameters.add("loop", new juce::AudioParameterFloat({ "fadelength", schema }, "Fade length", minFadeLength, maxFadeLength, 0.0f));
    parameters.add("loop", new juce::AudioParameterFloat({ "triggerrate", schema }, "Trigger rate", minTriggerRate, maxTriggerRate, 0.5f));
    parameters.add("loop", new juce::AudioParameterFloat({ "gaplength", schema }, "Gap length", minGapLength, maxGapLength, 0.5f));
    parameters.add("playbackorder", new juce::AudioParameterChoice({ "playbackorder", schema }, "Order", juce::StringArray(PlaybackOrder::labels), 0));
}

SamplerParameters::~SamplerParameters()
{
}

#pragma mark -

void SamplerParameters::onBypass(std::function<void()> callback)
{
    parameters.call("bypass", callback);
}

void SamplerParameters::onLoopMode(std::function<void()> callback)
{
    parameters.call("loop", callback);
}

void SamplerParameters::onPlaybackOrder(std::function<void()> callback)
{
    parameters.call("playbackorder", callback);
}

void SamplerParameters::onReset(std::function<void()> callback)
{
    parameters.call("reset", callback);
}

juce::AudioProcessorParameter* SamplerParameters::raw(const std::string& id) const
{
    return parameters.raw(id);
}

bool SamplerParameters::bypass() const
{
    return parameters.get<bool>("bypass");
}

LoopMode SamplerParameters::loopMode() const
{
    const std::string label = parameters.get<std::string>("loopmode");
    const LoopMode::Mode mode = (LoopMode::Mode) index_of(std::begin(LoopMode::labels), std::end(LoopMode::labels), label.c_str());
    
    switch (mode) {
            using enum LoopMode::Mode;
        case none:
            return LoopMode::none();
        case fade:
            return LoopMode::fade(loopFadeLength());
        case trigger:
            return LoopMode::trigger(loopTriggerRate());
        case gap:
            return LoopMode::gap(loopGapLength());
    }
}

float SamplerParameters::loopFadeLength() const
{
    return parameters.get<float>("fadelength");
}

float SamplerParameters::loopTriggerRate() const
{
    return parameters.get<float>("triggerrate");
}

float SamplerParameters::loopGapLength() const
{
    return parameters.get<float>("gaplength");
}

PlaybackOrder SamplerParameters::playbackOrder() const
{
    const std::string label = parameters.get<std::string>("playbackorder");
    const PlaybackOrder::Order order = (PlaybackOrder::Order) index_of(std::begin(PlaybackOrder::labels), std::end(PlaybackOrder::labels), label.c_str());
    
    switch (order) {
            using enum PlaybackOrder::Order;
        case ordinal:
            return PlaybackOrder::ordinal();
        case shuffle:
            return PlaybackOrder::shuffle();
        case random:
            return PlaybackOrder::random();
    }
}

#pragma mark -

void SamplerParameters::read(const void* data, const int size)
{
    try
    {
        auto xml = std::unique_ptr<juce::XmlElement>(juce::AudioProcessor::getXmlFromBinary(data, size));
        
        if (xml)
            DBG(xml->toString(juce::XmlElement::TextFormat().withoutHeader()));
        else
            return;
        
        if (xml->hasTagName("BanditexSampler") == false) { return; }
        if (xml->getIntAttribute("schema") != schema) { return; }
        
        parameters.read<bool>("bypass", *xml);
        parameters.read<std::string>("loopmode", *xml);
        parameters.read<float>("fadelength", *xml);
        parameters.read<float>("triggerrate", *xml);
        parameters.read<float>("gaplength", *xml);
        parameters.read<std::string>("playbackorder", *xml);
    }
    catch (const std::exception& exception)
    {
        juce::ignoreUnused(exception);
        DBG(exception.what());
    }
}

void SamplerParameters::write(juce::MemoryBlock& data)
{
    try
    {
        auto xml = std::make_unique<juce::XmlElement>("BanditexSampler");
        
        xml->setAttribute("schema", schema);
        
        parameters.write<bool>("bypass", *xml);
        parameters.write<std::string>("loopmode", *xml);
        parameters.write<float>("fadelength", *xml);
        parameters.write<float>("triggerrate", *xml);
        parameters.write<float>("gaplength", *xml);
        parameters.write<std::string>("playbackorder", *xml);
        
        DBG(xml->toString(juce::XmlElement::TextFormat().withoutHeader()));
        
        juce::AudioProcessor::copyXmlToBinary(*xml, data);
    }
    catch (const std::exception& exception)
    {
        juce::ignoreUnused(exception);
        DBG(exception.what());
    }
}
