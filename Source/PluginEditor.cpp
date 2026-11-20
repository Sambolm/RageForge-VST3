#include "PluginEditor.h"
RageForgeEditor::RageForgeEditor(RageForgeProcessor& p):AudioProcessorEditor(&p),processor(p){
 setSize(880,680);
 brand.setText("RAGEFORGE",juce::dontSendNotification);brand.setFont(juce::Font(juce::FontOptions(40.f,juce::Font::bold)));
 brand.setColour(juce::Label::textColourId,juce::Colour(0xffffb24f));addAndMakeVisible(brand);
 tagline.setText("BASS DESIGN ENGINE  /  ETERNATONES",juce::dontSendNotification);tagline.setFont(juce::Font(juce::FontOptions(13.f)));
 tagline.setColour(juce::Label::textColourId,juce::Colour(0xff95a7b7));addAndMakeVisible(tagline);
 subtitle.setText("Select a starting patch, play MIDI, then sculpt the sound.",juce::dontSendNotification);
 subtitle.setColour(juce::Label::textColourId,juce::Colours::lightgrey);addAndMakeVisible(subtitle);
 const char* names[]={"Impact Stab","Talking Monster","Cyber Metal","Void Reese","Riddim Chop","Liquid Growl","Laser Shot","Sub Engine"};
 for(int i=0;i<8;++i)preset.addItem(names[i],i+1);
 preset.setTextWhenNothingSelected("LOAD STARTING PATCH");
 preset.onChange=[this]{if(preset.getSelectedId()>0)processor.selectPreset(preset.getSelectedId()-1);};
 addAndMakeVisible(preset);
 const std::pair<const char*,const char*> controls[]{
  {"shape","WAVE MORPH"},{"fm","FM BITE"},{"detune","DETUNE"},{"sub","SUB LEVEL"},
  {"cutoff","FILTER"},{"resonance","RESONANCE"},{"vowel","VOCAL"},{"movement","MOVEMENT"},
  {"rate","LFO RATE"},{"drive","DRIVE"},{"attack","ATTACK"},{"decay","DECAY"},
  {"sustain","SUSTAIN"},{"release","RELEASE"},{"stereo","STEREO"},{"mix","EFFECT MIX"},
  {"glide","GLIDE"},{"level","OUTPUT"}
 };
 for(auto [id,label]:controls){
  auto k=std::make_unique<Knob>();
  k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,18);
  k->slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffffa73d));
  k->slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffffcc82));
  k->label.setText(label,juce::dontSendNotification);k->label.setJustificationType(juce::Justification::centred);
  k->label.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
  k->label.setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(k->slider);addAndMakeVisible(k->label);
  k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,k->slider);
  knobs.push_back(std::move(k));
 }
 addAndMakeVisible(mono);monoAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"mono",mono);
}
void RageForgeEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff0d121a));
 juce::ColourGradient grad(juce::Colour(0xff27374b),0,0,juce::Colour(0xff0d121a),0,140,false);
 g.setGradientFill(grad);g.fillRect(0,0,getWidth(),120);
 g.setColour(juce::Colour(0xff2c3748));g.drawHorizontalLine(130,22.f,858.f);
 g.setColour(juce::Colours::slategrey);g.setFont(12.f);
 g.drawText("OSCILLATORS",24,135,260,20,juce::Justification::centredLeft);
 g.drawText("FILTER / MOVEMENT",316,135,260,20,juce::Justification::centredLeft);
 g.drawText("ENVELOPE / MIX",612,135,260,20,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff3b4552));g.drawRoundedRectangle(16.f,160.f,848.f,432.f,12.f,1.f);
 g.setColour(juce::Colour(0xff9da7b4));g.setFont(12.f);
 g.drawText("4x oversampled synthesis · phase modulation · morphing oscillators · formant filter · dedicated sub",30,620,820,24,juce::Justification::centred);
}
void RageForgeEditor::resized(){
 brand.setBounds(24,14,320,58);tagline.setBounds(28,70,370,22);
 preset.setBounds(595,36,255,35);subtitle.setBounds(590,82,270,25);
 // 6 columns by 3 rows, 18 large knobs
 for(size_t i=0;i<knobs.size();++i){int row=static_cast<int>(i/6),col=static_cast<int>(i%6);
  int x=27+col*140,y=177+row*136;
  knobs[i]->label.setBounds(x,y,126,22);knobs[i]->slider.setBounds(x+7,y+18,112,102);
 }
 mono.setBounds(36,602,170,26);
}
