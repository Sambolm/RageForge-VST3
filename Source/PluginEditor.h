#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class RageForgeEditor final : public juce::AudioProcessorEditor {
public:
 explicit RageForgeEditor(RageForgeProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 RageForgeProcessor& processor;
 juce::ComboBox preset;
 juce::ComboBox lfoDivision, lfoShape;
 juce::Label lfoDivisionLabel, lfoShapeLabel;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment, shapeAttachment;
 juce::Label brand,tagline,subtitle;
 struct Knob {
  juce::Slider slider;
  juce::Label label;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
 };
 std::vector<std::unique_ptr<Knob>> knobs;
 juce::ToggleButton mono{"MONO / LEGATO"};
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageForgeEditor)
};
