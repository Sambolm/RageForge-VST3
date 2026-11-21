#include "PluginEditor.h"
void RageForgeEditor::NeonLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider){
 auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h);
 const float d=juce::jmin(bounds.getWidth(),bounds.getHeight())-20.f;
 const float radius=d*.5f;
 const auto centre=bounds.getCentre();
 const float angle=start+pos*(end-start);
 juce::Path track;
 track.addCentredArc(centre.x,centre.y,radius,radius,0.f,start,end,true);
 g.setColour(juce::Colour(0xff283442));
 g.strokePath(track,juce::PathStrokeType(7.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::Path arc;
 arc.addCentredArc(centre.x,centre.y,radius,radius,0.f,start,angle,true);
 const auto accent=juce::Colour(0xffff335b);
 g.setColour(accent.withAlpha(.16f));
 g.strokePath(arc,juce::PathStrokeType(13.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 g.setColour(accent);
 g.strokePath(arc,juce::PathStrokeType(6.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 g.setColour(juce::Colour(0xff101b29));
 g.fillEllipse(centre.x-radius*.77f,centre.y-radius*.77f,radius*1.54f,radius*1.54f);
 g.setColour(juce::Colour(0xff344b5d));
 g.drawEllipse(centre.x-radius*.77f,centre.y-radius*.77f,radius*1.54f,radius*1.54f,1.5f);
 juce::Path indicator;
 indicator.addLineSegment({centre.x+std::sin(angle)*radius*.35f,centre.y-std::cos(angle)*radius*.35f,
                           centre.x+std::sin(angle)*radius*.68f,centre.y-std::cos(angle)*radius*.68f},3.f);
 g.setColour(juce::Colour(0xff72ecfa));
 g.strokePath(indicator,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::ignoreUnused(slider);
}
void RageForgeEditor::NeonLookAndFeel::drawComboBox(juce::Graphics& g,int width,int height,bool isDown,int,int,int,int,juce::ComboBox& box){
 auto rect=juce::Rectangle<float>(0.f,0.f,(float)width,(float)height).reduced(1.f);
 g.setColour(juce::Colour(isDown?0xff21334a:0xff152637));g.fillRoundedRectangle(rect,7.f);
 g.setColour(juce::Colour(0xff356275));g.drawRoundedRectangle(rect,7.f,1.f);
 juce::Path arrow;
 float cx=(float)width-16.f,cy=(float)height*.5f;
 arrow.startNewSubPath(cx-5.f,cy-2.f);arrow.lineTo(cx,cy+3.f);arrow.lineTo(cx+5.f,cy-2.f);
 g.setColour(juce::Colour(0xff72ecfa));g.strokePath(arrow,juce::PathStrokeType(2.f));
 juce::ignoreUnused(box);
}
RageForgeEditor::RageForgeEditor(RageForgeProcessor& p):AudioProcessorEditor(&p),processor(p){
 setLookAndFeel(&neon);

 brand.setText("RAGEFORGE",juce::dontSendNotification);brand.setFont(juce::Font(juce::FontOptions(40.f,juce::Font::bold)));
 brand.setColour(juce::Label::textColourId,juce::Colour(0xffff335b));addAndMakeVisible(brand);
 tagline.setText("DUBSTEP SYNTHESIS SYSTEM  /  ETERNATONES",juce::dontSendNotification);tagline.setFont(juce::Font(juce::FontOptions(13.f)));
 tagline.setColour(juce::Label::textColourId,juce::Colour(0xff7ce8f8));addAndMakeVisible(tagline);
 subtitle.setText("Select a starting patch, play MIDI, then sculpt the sound.",juce::dontSendNotification);
 subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffb5d1df));addAndMakeVisible(subtitle);
 const char* names[]={"Impact Stab","Talking Monster","Cyber Metal","Void Reese","Riddim Chop","Liquid Growl","Laser Shot","Sub Engine"};
 for(int i=0;i<8;++i)preset.addItem(names[i],i+1);
 preset.setTextWhenNothingSelected("LOAD STARTING PATCH");
 preset.onChange=[this]{ if(preset.getSelectedId()>0){ processor.selectPreset(preset.getSelectedId()-1); subtitle.setText("LOADED: "+preset.getText()+"  |  Play a new note to audition.",juce::dontSendNotification); }};
 addAndMakeVisible(preset);
 const std::pair<const char*,const char*> controls[]{
  {"shape","WAVE MORPH"},{"fm","FM BITE"},{"detune","DETUNE"},{"sub","SUB LEVEL"},
  {"cutoff","FILTER"},{"resonance","RESONANCE"},{"vowel","VOCAL"},{"movement","MOVEMENT"},
  {"drive","DRIVE"},{"attack","ATTACK"},{"decay","DECAY"},
  {"sustain","SUSTAIN"},{"release","RELEASE"},{"stereo","STEREO"},{"mix","EFFECT MIX"},
  {"glide","GLIDE"},{"level","OUTPUT"}
 };
 for(auto [id,label]:controls){
  auto k=std::make_unique<Knob>();
  k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,74,20);
  const juce::String idString(id);
  // No long decimal strings. Envelopes retain millisecond-level editing accuracy.
  k->slider.setNumDecimalPlacesToDisplay(idString=="cutoff"?0:
     (idString=="attack"||idString=="decay"||idString=="release"?3:2));
  k->slider.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffc9f7ff));
  k->slider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff101d2a));
  k->slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff254558));
  k->slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffff335b));
  k->slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xff72ecfa));
  k->label.setText(label,juce::dontSendNotification);k->label.setJustificationType(juce::Justification::centred);
  k->label.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
  k->label.setColour(juce::Label::textColourId,juce::Colours::lightgrey);
  addAndMakeVisible(k->slider);addAndMakeVisible(k->label);
  k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,id,k->slider);
  knobs.push_back(std::move(k));
 }
 const juce::StringArray divisions{"1/32","1/16","1/8","1/4","1/2","1 BAR","2 BARS"};
 const juce::StringArray shapes{"Sine","Triangle","Saw Up","Saw Down","Square"};
 lfoDivision.addItemList(divisions,1);
 lfoShape.addItemList(shapes,1);
 lfoDivisionLabel.setText("LFO SYNC",juce::dontSendNotification);
 lfoShapeLabel.setText("LFO SHAPE",juce::dontSendNotification);
 for(auto* label:{&lfoDivisionLabel,&lfoShapeLabel}){
   label->setJustificationType(juce::Justification::centred);
   label->setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
   label->setColour(juce::Label::textColourId,juce::Colours::lightgrey);
   addAndMakeVisible(*label);
 }
 for(auto* box:{&lfoDivision,&lfoShape}){
   box->setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff152637));
   box->setColour(juce::ComboBox::textColourId,juce::Colours::white);
   addAndMakeVisible(*box);
 }
 divisionAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"lfoDivision",lfoDivision);
 shapeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"lfoShape",lfoShape);
 addAndMakeVisible(mono);monoAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"mono",mono);
 neon.setColour(juce::ComboBox::textColourId,juce::Colour(0xffe5f7ff));
 neon.setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff101b29));
 neon.setColour(juce::PopupMenu::textColourId,juce::Colour(0xffe5f7ff));
 // Size the editor only AFTER all controls exist, so resized() positions every knob.
 setSize(880,680);
}
RageForgeEditor::~RageForgeEditor(){setLookAndFeel(nullptr);}
void RageForgeEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff080e17));
 juce::ColourGradient head(juce::Colour(0xff19293d),0.f,0.f,juce::Colour(0xff080e17),0.f,135.f,false);
 g.setGradientFill(head);g.fillRect(0,0,getWidth(),137);
 g.setColour(juce::Colour(0x20ff335b));
 for(int x=0;x<getWidth();x+=29)g.drawVerticalLine(x,3.f,124.f);
 g.setColour(juce::Colour(0xfffa355c));g.fillRoundedRectangle(20.f,115.f,838.f,2.f,1.f);
 g.setColour(juce::Colour(0xff183045));g.fillRoundedRectangle(15.f,157.f,850.f,440.f,13.f);
 g.setColour(juce::Colour(0xff31546a));g.drawRoundedRectangle(15.f,157.f,850.f,440.f,13.f,1.2f);
 g.setColour(juce::Colour(0xff0b1826));
 for(int row=0;row<3;++row)for(int col=0;col<6;++col)
    g.fillRoundedRectangle((float)(23+col*140),(float)(170+row*136),134.f,126.f,8.f);
 g.setColour(juce::Colour(0xff64dfe9));g.setFont(juce::Font(juce::FontOptions(12.f,juce::Font::bold)));
 g.drawText("OSCILLATORS  /  FM",26,135,275,18,juce::Justification::centredLeft);
 g.drawText("MODULATION  /  TONE",307,135,275,18,juce::Justification::centredLeft);
 g.drawText("ENVELOPES  /  OUTPUT",606,135,260,18,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff4a788b));g.drawVerticalLine(302,160.f,590.f);g.drawVerticalLine(586,160.f,590.f);
 g.setColour(juce::Colour(0xff96b4c5));g.setFont(12.f);
 g.drawText("RAGEFORGE  //  4x OVERSAMPLING  //  FM + FORMANT  //  SUB ENGINE",222,614,630,26,juce::Justification::centredRight);
}
void RageForgeEditor::resized(){
 brand.setBounds(24,14,320,58);tagline.setBounds(28,70,370,22);
 preset.setBounds(595,36,255,35);subtitle.setBounds(590,82,270,25);
 // 6 columns by 3 rows. The two LFO selectors occupy the last cell.
 for(size_t i=0;i<knobs.size();++i){int row=static_cast<int>(i/6),col=static_cast<int>(i%6);
  int x=27+col*140,y=177+row*136;
  knobs[i]->label.setBounds(x,y,126,22);knobs[i]->slider.setBounds(x+7,y+18,112,102);
 }
 lfoDivisionLabel.setBounds(726,453,126,18);
 lfoDivision.setBounds(727,472,124,28);
 lfoShapeLabel.setBounds(726,511,126,18);
 lfoShape.setBounds(727,530,124,28);
 mono.setBounds(36,602,170,26);
}
