
#include "PluginEditor.h"


PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), pluginProcessor(p),
    globalParams(new juce::GenericAudioProcessorEditor(p)),
    inspectButton(new juce::TextButton("Inspect the UI"))
{
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
    
    setSize (500, 600);
    setResizable(true, true);
    setResizeLimits(400, 400, 1000, 1000);
}

PluginEditor::~PluginEditor()
{
}

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
