
#include "PluginEditor.h"


PluginEditor::PluginEditor(PluginProcessor& p)
    : juce::AudioProcessorEditor(&p), pluginProcessor(p),
    globalParams(new juce::GenericAudioProcessorEditor(p)),
    inspectButton(new juce::TextButton("Inspect the UI"))
{
    pluginProcessor.mainProcessor->addChangeListener(this);
    
    headerComp.addAndMakeVisible(*globalParams);
    headerComp.addAndMakeVisible(*inspectButton);
    addAndMakeVisible(headerComp);
    
    inspectButton->onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }
        
        inspector->setVisible (true);
    };
    
    for (auto node : pluginProcessor.mainProcessor->getNodes())
        if (node->getProcessor()->hasEditor())
            procComp.addAndMakeVisible(node->getProcessor()->createEditor());
    addAndMakeVisible(procComp);
    
    setSize (600, 800);
}

PluginEditor::~PluginEditor()
{
    pluginProcessor.mainProcessor->removeChangeListener(this);
    procComp.deleteAllChildren();
}

#pragma mark -

void PluginEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    //TODO: here we can catch AudioProcessorGraph changes
    juce::ignoreUnused(source);
}

#pragma mark -

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    
    headerComp.setBounds(area.removeFromTop(40));
    globalParams->setBounds(headerComp.getLocalBounds().removeFromLeft(area.getCentreX()));
    inspectButton->setBounds(headerComp.getLocalBounds().removeFromRight(area.getCentreX()).reduced(10));
    
    procComp.setBounds(area);
    auto procArea = procComp.getLocalBounds();
    for (auto component : procComp.getChildren())
        component->setBounds(procArea.removeFromTop(component->getHeight()));
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
