#pragma once
#include <JuceHeader.h>
#include "RageDSP.h"

class RageForgeProcessor final : public juce::AudioProcessor {
public:
 RageForgeProcessor();
 ~RageForgeProcessor() override = default;
 const juce::String getName() const override {return "RageForge";}
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 3.0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
 void selectPreset(int);
 juce::AudioProcessorValueTreeState state;
private:
 rage::Engine engine;
 void syncParameters();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageForgeProcessor)
};
