/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NVS_EQAudioProcessor::NVS_EQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

NVS_EQAudioProcessor::~NVS_EQAudioProcessor()
{
}

//==============================================================================
const juce::String NVS_EQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NVS_EQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NVS_EQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NVS_EQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NVS_EQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NVS_EQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NVS_EQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NVS_EQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NVS_EQAudioProcessor::getProgramName (int index)
{
    return {};
}

void NVS_EQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NVS_EQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // TODO - what does this code do?
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    // we are using two mono channels, but the processing chain is a mono processing chain
    spec.numChannels = 1;
    
    LeftChain.prepare(spec);
    RightChain.prepare(spec);
    
    // now lets
    auto chainSettings = getChainSettings(apvts);
    
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQ, juce::Decibels::decibelsToGain( chainSettings.peakGain));

    /*
    auto LCFCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, chainSettings.lowCutFreq, chainSettings.lowCutSlope);
    
    auto HCFCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, chainSettings.highCutFreq, chainSettings.highCutSlope);*/
    
    // TODO - pretty sure that this should be done within a GUI-rate function instead of audio-rate...
    // can this be moved to paint or something instead?
    *LeftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    /*
    *LeftChain.get<ChainPositions::HighCut>().coefficients = *HCFCoefficients;
    *LeftChain.get<ChainPositions::LowCut>().coefficients = *LCFCoefficients;*/
    
    *RightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    /*
    *RightChain.get<ChainPositions::HighCut>().coefficients = *HCFCoefficients;
    *RightChain.get<ChainPositions::LowCut>().coefficients = *LCFCoefficients;*/
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, (chainSettings.lowCutSlope + 1) * 2);
    
    auto& leftLowCut = LeftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;
        }
    }
    
    auto& rightLowCut = RightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);
    
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;
        }
    }

}

void NVS_EQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NVS_EQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NVS_EQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // now lets
    auto chainSettings = getChainSettings(apvts);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQ, juce::Decibels::decibelsToGain( chainSettings.peakGain));
    
    /*
    auto LCFCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), chainSettings.lowCutFreq, chainSettings.lowCutSlope);
    auto HCFCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), chainSettings.highCutFreq, chainSettings.highCutSlope);
    */
    // TODO - pretty sure that this should be done within a GUI-rate function instead of audio-rate...
    // can this be moved to paint or something instead?
    *LeftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    /*
    *LeftChain.get<ChainPositions::HighCut>().coefficients = *HCFCoefficients;
    *LeftChain.get<ChainPositions::LowCut>().coefficients = *LCFCoefficients;*/
    
    *RightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    /*
    *RightChain.get<ChainPositions::HighCut>().coefficients = *HCFCoefficients;
    *RightChain.get<ChainPositions::LowCut>().coefficients = *LCFCoefficients;*/
    
    auto cutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), (chainSettings.lowCutSlope + 1) * 2);
    
    auto& leftLowCut = LeftChain.get<ChainPositions::LowCut>();
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *leftLowCut.get<0>().coefficients = *cutCoefficients[0];
            leftLowCut.setBypassed<0>(false);
            *leftLowCut.get<1>().coefficients = *cutCoefficients[1];
            leftLowCut.setBypassed<1>(false);
            *leftLowCut.get<2>().coefficients = *cutCoefficients[2];
            leftLowCut.setBypassed<2>(false);
            *leftLowCut.get<3>().coefficients = *cutCoefficients[3];
            leftLowCut.setBypassed<3>(false);
            break;
        }
    }
    
    auto& rightLowCut = RightChain.get<ChainPositions::LowCut>();
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);
    
    switch (chainSettings.lowCutSlope) {
        case Slope_12:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            break;
        }
        case Slope_24:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            break;
        }
        case Slope_36:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            break;
        }
        case Slope_48:
        {
            *rightLowCut.get<0>().coefficients = *cutCoefficients[0];
            rightLowCut.setBypassed<0>(false);
            *rightLowCut.get<1>().coefficients = *cutCoefficients[1];
            rightLowCut.setBypassed<1>(false);
            *rightLowCut.get<2>().coefficients = *cutCoefficients[2];
            rightLowCut.setBypassed<2>(false);
            *rightLowCut.get<3>().coefficients = *cutCoefficients[3];
            rightLowCut.setBypassed<3>(false);
            break;
        }
    }


    
    // TODO - what does this code really do? add better notes!
    // block is initialised with our current audio buffer
    // create an audio block which can be sent to our processes
    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    // now we can pass these contexts to our mono filter chains...
    LeftChain.process(leftContext);
    RightChain.process(rightContext);
    
    
}

//==============================================================================
bool NVS_EQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NVS_EQAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void NVS_EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NVS_EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.lowCutSlope = static_cast<Slope>( apvts.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>( apvts.getRawParameterValue("HighCut Slope")->load());
    settings.peakGain = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQ = apvts.getRawParameterValue("Peak Q")->load();
    
    return settings;
}


juce::AudioProcessorValueTreeState::ParameterLayout
    NVS_EQAudioProcessor::createParameterLayout()
{
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        
        // frequency for the EQ
        layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq",
             juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));
        
        layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.25f), 20000.f));
        
        layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.38f), 1000.f));
        
        // gain amount for the EQ
        
        // string array for the low and highcut EQ options
        
        juce::StringArray stringArray;
        for (int i = 0; i < 4; ++i)
        {
            juce::String str;
            str << (12 + i * 12);
            str << "db/Oct";
            stringArray.add(str);
        }
        
        layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));

        layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));
        
        // gain amount for the EQ
        layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
        
        // gain amount for the EQ
        layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Q", "Peak Q", juce::NormalisableRange<float>(0.1f, 10.f, 0.005f, 1.f), 0.7f));
        
        return layout;
    }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NVS_EQAudioProcessor();
}


