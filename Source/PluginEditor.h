/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};


struct ResponseCurveComponent: juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    
        ResponseCurveComponent(NVS_EQAudioProcessor&);
        ~ResponseCurveComponent();
       
        void parameterValueChanged (int parameterIndex, float newValue) override;
        // we dont care about gestures, so we can leave this function blank
        void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { };

        void timerCallback() override;
        
        void paint(juce::Graphics&) override;
        
    private:
        NVS_EQAudioProcessor& audioProcessor;
        // need an atomic flag to do something ? TODO
        juce::Atomic<bool> parametersChanged { false };
        
        MonoChain monoChain;
};

//==============================================================================
/**
*/
class NVS_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NVS_EQAudioProcessorEditor (NVS_EQAudioProcessor&);
    ~NVS_EQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    
private:
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NVS_EQAudioProcessor& audioProcessor;
    
    CustomRotarySlider peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider, highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    ResponseCurveComponent responseCurveComponent;
    
    Attachment peakFreqSliderAttachment, peakGainSliderAttachment, peakQualitySliderAttachment, lowCutFreqSliderAttachment, highCutFreqSliderAttachment, lowCutSlopeSliderAttachment, highCutSlopeSliderAttachment;
    
    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NVS_EQAudioProcessorEditor)
};
