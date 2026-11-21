#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class RageForgeEditor final : public juce::AudioProcessorEditor {
public:
 explicit RageForgeEditor(RageForgeProcessor&);
 ~RageForgeEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 class NeonLookAndFeel final : public juce::LookAndFeel_V4 {
 public:
  void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
  void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
  void drawLinearSlider(juce::Graphics&, int,int,int,int,float,float,float,juce::Slider::SliderStyle,juce::Slider&) override;
 } neon;
 RageForgeProcessor& processor;
 class WaveDraw final : public juce::Component, public juce::SettableTooltipClient {
 public:
  explicit WaveDraw(RageForgeProcessor& p):processor(p){}
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
 private:
  RageForgeProcessor& processor;
  int lastIndex=-1;float lastValue=0.f;
  void edit(const juce::MouseEvent&);
 };
 WaveDraw waveDrawing;
 juce::Slider waveBlendSlider,waveWarpSlider;
 juce::Label waveBlendValue,waveWarpValue;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveBlendAttachment,waveWarpAttachment;
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
