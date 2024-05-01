
#include "PluginParameters.h"


PluginParameters::PluginParameters(juce::AudioProcessor& proc)
    : parameters(proc)
{
    parameters.add("bypass", new juce::AudioParameterBool({ "bypass", schema }, "Bypass", false, juce::AudioParameterBoolAttributes()));
    parameters.add("numsamplers", new juce::AudioParameterInt({ "numsamplers", schema }, "Number of samplers", 1, maxsamplers, 1));
}

PluginParameters::~PluginParameters()
{
}

void PluginParameters::onbypass(std::function<void ()> callback)
{
    parameters.call("bypass", callback);
}

void PluginParameters::onnumsamplers(std::function<void ()> callback)
{
    parameters.call("numsamplers", callback);
}

void PluginParameters::onreset(std::function<void ()> callback)
{
    parameters.call("reset", callback);
}

juce::AudioProcessorParameter* PluginParameters::raw(const std::string& id) const
{
    return parameters.raw(id);
}

bool PluginParameters::bypass() const
{
    return parameters.get<bool>("bypass");
}

int PluginParameters::numsamplers(const int newnumsamplers) const
{
    return std::max(newnumsamplers, maxsamplers);
}

#pragma mark -

void PluginParameters::read(const void* data, const int size)
{
    try
    {
        auto xml = std::unique_ptr<juce::XmlElement>(juce::AudioProcessor::getXmlFromBinary(data, size));
        
        if (xml)
            DBG(xml->toString(juce::XmlElement::TextFormat().withoutHeader()));
        else
            return;
        
        if (xml->hasTagName("BanditexPlugin") == false) { return; }
        if (xml->getIntAttribute("schema") != schema) { return; }
        
        parameters.read<bool>("bypass", *xml);
        parameters.read<int>("numsamplers", *xml);
    }
    catch(const std::exception& exception)
    {
        juce::ignoreUnused(exception);
        DBG(exception.what());
    }
}

void PluginParameters::write(juce::MemoryBlock& data)
{
    try
    {
        auto xml = std::make_unique<juce::XmlElement>("BanditexPlugin");
        xml->setAttribute("schema", schema);
        
        parameters.write<bool>("bypass", *xml);
        parameters.write<int>("numsamplers", *xml);
        
        DBG(xml->toString(juce::XmlElement::TextFormat().withoutHeader()));
        juce::AudioProcessor::copyXmlToBinary(*xml, data);
    }
    catch(const std::exception& exception)
    {
        juce::ignoreUnused(exception);
        DBG(exception.what());
    }
}
