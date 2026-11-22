#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class RageForgeEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
 explicit RageForgeEditor(RageForgeProcessor&);
 ~RageForgeEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
 int getControlParameterIndex(juce::Component&) override;
private:
 void showAutomationMenu(const juce::String& parameterId);
 void timerCallback() override;
 bool playheadPreviouslyVisible=false;
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
  void mouseUp(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
 private:
  RageForgeProcessor& processor;
  int lastIndex=-1,selectedGrid=-1;
  float lastValue=0.f;
  juce::Point<float> pointerDown{};
  bool isDragging=false,skipClick=false;
  void editAt(juce::Point<float>);
  void placeGridPoint(juce::Point<float>);
 };
 WaveDraw waveDrawing;
 class AutomatableSlider final : public juce::Slider {
 public:
  std::function<void()> onContextClick;
  void mouseDown(const juce::MouseEvent& e) override {
   if(e.mods.isPopupMenu()) {
    if(onContextClick)onContextClick();
    return;
   }
   juce::Slider::mouseDown(e);
  }
  void mouseDrag(const juce::MouseEvent& e) override {
   if(e.mods.isPopupMenu())return;
   juce::Slider::mouseDrag(e);
  }
  void mouseUp(const juce::MouseEvent& e) override {
   if(e.mods.isPopupMenu())return;
   juce::Slider::mouseUp(e);
  }
 };
 AutomatableSlider waveBlendSlider,waveWarpSlider;
 juce::Label waveBlendValue,waveWarpValue;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveBlendAttachment,waveWarpAttachment;
 juce::ComboBox preset,dropPreset;
 juce::TextButton dropPageButton{"DROP LAB"};
 juce::TextButton generateBassButton{"GENERATE VARIATION"};
 juce::TextButton generateRhythmButton{"GENERATE RHYTHM"};
 juce::ToggleButton rhythmEnabled{"RHYTHM ON"};
 juce::ComboBox rhythmGrid,rhythmDensity;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rhythmGridAttachment,rhythmDensityAttachment;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> rhythmEnabledAttachment;
 int generatedVariations=0;
 bool dropLabPage=false;
 void updatePage();
 juce::ComboBox lfoDivision, lfoShape;
 juce::Label lfoDivisionLabel, lfoShapeLabel;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment, shapeAttachment;
 juce::Label brand,tagline,subtitle;
 struct Knob {
  AutomatableSlider slider;
  juce::String parameterId;
  juce::Label label;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
 };
 std::vector<std::unique_ptr<Knob>> knobs;
 juce::ToggleButton mono{"MONO / LEGATO"};
 juce::ToggleButton resetEachNote{"RESET EACH NOTE"};
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monoAttachment,resetEachNoteAttachment;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageForgeEditor)
};
