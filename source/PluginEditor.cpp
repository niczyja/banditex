
#include "PluginEditor.h"


PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p),
    globalParams(new juce::GenericAudioProcessorEditor(p)),
    inspectButton(new juce::TextButton("Inspect the UI"))
{
    header.addAndMakeVisible(*globalParams);
    header.addAndMakeVisible(*inspectButton);
    addAndMakeVisible(header);
    
    inspectButton->onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }
        
        inspector->setVisible (true);
    };
    
    for (auto node : p.mainProcessor->getNodes())
        if (node->getProcessor()->hasEditor())
            processors.addAndMakeVisible(node->getProcessor()->createEditor());
    addAndMakeVisible(processors);
    
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
    
    header.setBounds(area.removeFromTop(40));
    globalParams->setBounds(header.getLocalBounds().removeFromLeft(area.getCentreX()));
    inspectButton->setBounds(header.getLocalBounds().removeFromRight(area.getCentreX()).reduced(10));
    
    processors.setBounds(area);
    auto procArea = processors.getLocalBounds();
    for (auto procComp : processors.getChildren())
        procComp->setBounds(procArea.removeFromTop(procComp->getHeight()));
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
