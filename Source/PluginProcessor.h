/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
  Slope_12,
  Slope_24,
  Slope_36,
  Slope_48
};

struct ChainSettings
{
    float lowCutFreq{0};
    float highCutFreq{0};
    float peakFreq{1.f};
    int lowCutSlope{Slope::Slope_12};
    int highCutSlope{Slope::Slope_12};
    float peakGain{0};
    float peakQ{0};
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);
//==============================================================================
/**
*/
class NVS_EQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NVS_EQAudioProcessor();
    ~NVS_EQAudioProcessor() override;

    //==============================================================================
    // This is called by the plugin right before the plugin is about to play
    // This is where any setup and initialization needs to occur
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    // this is the audio-rate processing that occurs within the plugin
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // this function syncing the values of GUI objects and variables within the plugin
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    
    // this can be static as it does not use any member variables
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters",
        createParameterLayout()};

    
private:
    // create an alies for this datatype that is more human-readable
    using Filter = juce::dsp::IIR::Filter<float>;
    
    // need 4x of these filters in a processing chain for the low-pass filter, high-pass filter, and other parameters for our EQ
    using QuadFilterProcessingChain = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    // we need to create a chain for each channel of audio =)
    using MonoChain = juce::dsp::ProcessorChain<QuadFilterProcessingChain, Filter, QuadFilterProcessingChain>;
    
    // create two instances of MonoChain
    MonoChain LeftChain, RightChain;
    
    enum ChainPositions
    {
        LowCut, Peak, HighCut
    };
    
    void updatePeakFilter(const ChainSettings &chainSettings);
    void updateHCFilter(const ChainSettings &chainSettings);
    void updateLCFilter(const ChainSettings &chainSettings);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NVS_EQAudioProcessor)
};
