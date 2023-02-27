/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NVS_EQAudioProcessorEditor::NVS_EQAudioProcessorEditor (NVS_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // the the vector of slider components to iterate over all GUI components in a for loop
    for (auto* comp : getComps()) {
        addAndMakeVisible(comp);
    }
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

NVS_EQAudioProcessorEditor::~NVS_EQAudioProcessorEditor()
{
}

//==============================================================================
void NVS_EQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NVS_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    
    // remove the top third of the screen which will be used to generate the EQ graph
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    // now remove the left third which will be used for low-cut
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    // what is left of bounds is now our peak area
    auto peakArea = bounds;
    
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.66));
    lowCutSlopeSlider.setBounds(lowCutArea);
    
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.66));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakFreqSlider.setBounds(peakArea.removeFromTop(peakArea.getHeight()*0.33));
    peakGainSlider.setBounds(peakArea.removeFromTop(peakArea.getHeight()*0.5));
    peakQualitySlider.setBounds(peakArea);
    // create vector to allow for easy iteration over these similar GUI components
}

std::vector<juce::Component*> NVS_EQAudioProcessorEditor::getComps() {
    return
    {
      &peakFreqSlider,
      &peakGainSlider,
      &peakQualitySlider,
      &lowCutFreqSlider,
      &highCutFreqSlider,
      &lowCutSlopeSlider,
      &highCutSlopeSlider
    };
}
