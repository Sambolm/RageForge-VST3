#pragma once
#include <JuceHeader.h>
#include "RageDSP.h"
#include <atomic>
#include <array>

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
 void selectDropPreset(int);
 void generateDropVariation(int family);
 void setWavePoint(int index,float value);
 float getWavePoint(int index) const;
 bool isWavePlaying() const noexcept {return wavePlaying.load(std::memory_order_relaxed);}
 float getWavePhase() const noexcept {return wavePhase.load(std::memory_order_relaxed);}
 float getWaveScopeProgress() const noexcept {return waveScopeProgress.load(std::memory_order_relaxed);}
 juce::AudioProcessorValueTreeState state;
private:
 rage::Engine engine;
 std::array<std::atomic<float>,rage::Engine::waveSize> userWave{};
 uint32_t variationCounter=0;
 std::atomic<bool> wavePlaying{false};
 std::atomic<float> wavePhase{0.f},waveScopeProgress{0.f};
 void syncParameters();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageForgeProcessor)
};
