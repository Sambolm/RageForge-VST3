#include "PluginEditor.h"
void RageForgeEditor::NeonLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float start,float end,juce::Slider& slider){
 auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h);
 const float d=juce::jmin(bounds.getWidth(),bounds.getHeight())-20.f;
 const float radius=d*.5f;
 const auto centre=bounds.getCentre();
 const float angle=start+pos*(end-start);
 juce::Path track;
 track.addCentredArc(centre.x,centre.y,radius,radius,0.f,start,end,true);
 g.setColour(juce::Colour(0xff1f3521));
 g.strokePath(track,juce::PathStrokeType(7.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::Path arc;
 arc.addCentredArc(centre.x,centre.y,radius,radius,0.f,start,angle,true);
 const auto accent=juce::Colour(0xff76ff29);
 g.setColour(accent.withAlpha(.16f));
 g.strokePath(arc,juce::PathStrokeType(13.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 g.setColour(accent);
 g.strokePath(arc,juce::PathStrokeType(6.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 g.setColour(juce::Colour(0xff0d180f));
 g.fillEllipse(centre.x-radius*.77f,centre.y-radius*.77f,radius*1.54f,radius*1.54f);
 g.setColour(juce::Colour(0xff426b3c));
 g.drawEllipse(centre.x-radius*.77f,centre.y-radius*.77f,radius*1.54f,radius*1.54f,1.5f);
 juce::Path indicator;
 indicator.addLineSegment({centre.x+std::sin(angle)*radius*.35f,centre.y-std::cos(angle)*radius*.35f,
                           centre.x+std::sin(angle)*radius*.68f,centre.y-std::cos(angle)*radius*.68f},3.f);
 g.setColour(juce::Colour(0xffb5ff6a));
 g.strokePath(indicator,juce::PathStrokeType(3.f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
 juce::ignoreUnused(slider);
}
void RageForgeEditor::NeonLookAndFeel::drawComboBox(juce::Graphics& g,int width,int height,bool isDown,int,int,int,int,juce::ComboBox& box){
 auto rect=juce::Rectangle<float>(0.f,0.f,(float)width,(float)height).reduced(1.f);
 g.setColour(juce::Colour(isDown?0xff203723:0xff101f13));g.fillRoundedRectangle(rect,7.f);
 g.setColour(juce::Colour(0xff497c3c));g.drawRoundedRectangle(rect,7.f,1.f);
 juce::Path arrow;
 float cx=(float)width-16.f,cy=(float)height*.5f;
 arrow.startNewSubPath(cx-5.f,cy-2.f);arrow.lineTo(cx,cy+3.f);arrow.lineTo(cx+5.f,cy-2.f);
 g.setColour(juce::Colour(0xffb5ff6a));g.strokePath(arrow,juce::PathStrokeType(2.f));
 juce::ignoreUnused(box);
}
RageForgeEditor::RageForgeEditor(RageForgeProcessor& p):AudioProcessorEditor(&p),processor(p){
 setLookAndFeel(&neon);

 brand.setText("RAGEFORGE",juce::dontSendNotification);brand.setFont(juce::Font(juce::FontOptions("Impact",53.f,juce::Font::bold)));
 brand.setColour(juce::Label::textColourId,juce::Colour(0xff76ff29));addAndMakeVisible(brand);
 tagline.setText("DUBSTEP BASS SYNTHESIZER  //  SIK RAGE PRODUCTIONS",juce::dontSendNotification);tagline.setFont(juce::Font(juce::FontOptions(13.f)));
 tagline.setColour(juce::Label::textColourId,juce::Colour(0xffb5ff6a));addAndMakeVisible(tagline);
 subtitle.setText("WAVETABLE  /  FM  /  FORMANT  /  SUB",juce::dontSendNotification);
 subtitle.setColour(juce::Label::textColourId,juce::Colour(0xffb5dbaa));addAndMakeVisible(subtitle);
 const char* names[]={"Impact Stab","Talking Monster","Cyber Metal","Void Reese","Riddim Chop","Liquid Growl","Laser Shot","Sub Engine"};
 for(int i=0;i<8;++i)preset.addItem(names[i],i+1);
 preset.setTextWhenNothingSelected("SELECT BASS PRESET");
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
  k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,80,20);
  const juce::String idString(id);
  // No long decimal strings. Envelopes retain millisecond-level editing accuracy.
  k->slider.setNumDecimalPlacesToDisplay(2);
  // JUCE's number-of-decimals hint alone is insufficient for certain APVTS ranges.
  // Explicitly format every editor readout as a fixed, two-decimal string.
  k->slider.textFromValueFunction=[](double v){return juce::String(v,2);};
  k->slider.valueFromTextFunction=[](const juce::String& text){return text.getDoubleValue();};
  k->slider.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffdcffcd));
  k->slider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff071008));
  k->slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff345936));
  k->slider.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff76ff29));
  k->slider.setColour(juce::Slider::thumbColourId,juce::Colour(0xffb5ff6a));
  k->label.setText(label,juce::dontSendNotification);k->label.setJustificationType(juce::Justification::centred);k->label.setMinimumHorizontalScale(.65f);
  k->label.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
  k->label.setColour(juce::Label::textColourId,juce::Colour(0xffc8f9a4));
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
   box->setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff101f13));
   box->setColour(juce::ComboBox::textColourId,juce::Colours::white);
   addAndMakeVisible(*box);
 }
 divisionAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"lfoDivision",lfoDivision);
 shapeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.state,"lfoShape",lfoShape);
 addAndMakeVisible(mono);monoAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"mono",mono);
 neon.setColour(juce::ComboBox::textColourId,juce::Colour(0xffeaffd9));
 neon.setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff0d180f));
 neon.setColour(juce::PopupMenu::textColourId,juce::Colour(0xffeaffd9));
 // Size the editor only AFTER all controls exist, so resized() positions every knob.
 for(auto &k:knobs) k->slider.onValueChange=[this]{repaint();};
 setResizable(true,true);
 setResizeLimits(740,548,1450,1074);
 setSize(890,659);
}
RageForgeEditor::~RageForgeEditor(){setLookAndFeel(nullptr);}
void RageForgeEditor::paint(juce::Graphics& g){
 const auto lime=juce::Colour(0xff76ff29), glow=juce::Colour(0xffb5ff6a);
 const auto bg=juce::Colour(0xff050905);
 g.fillAll(bg);
 // Draw on a 1080x800 virtual canvas, fitted to the actual plugin window.
 // FL Studio can now resize RageForge without hiding its bottom row.
 const float scale=juce::jmin((float)getWidth()/1080.f,(float)getHeight()/800.f);
 const float ox=((float)getWidth()-1080.f*scale)*.5f;
 const float oy=((float)getHeight()-800.f*scale)*.5f;
 g.addTransform(juce::AffineTransform::scale(scale).translated(ox,oy));
 juce::ColourGradient background(juce::Colour(0xff152416),540.f,0.f,bg,540.f,800.f,false);
 g.setGradientFill(background);g.fillRect(0,0,1080,800);
 // Slanted industrial header and electric-green masthead.
 juce::Path header;
 header.startNewSubPath(0.f,0.f);header.lineTo(1080.f,0.f);header.lineTo(1080.f,107.f);
 header.lineTo(1035.f,119.f);header.lineTo(0.f,119.f);header.closeSubPath();
 g.setColour(juce::Colour(0xff0a130b));g.fillPath(header);
 g.setColour(lime.withAlpha(.12f));
 for(int x=0;x<1080;x+=40)g.drawLine((float)x,0.f,(float)x+65.f,119.f,1.f);
 g.setColour(lime.withAlpha(.25f));g.drawLine(0.f,119.f,1080.f,119.f,2.f);
 g.setColour(lime.withAlpha(.13f));g.fillRoundedRectangle(20.f,17.f,480.f,78.f,6.f);
 g.setColour(lime.withAlpha(.45f));g.drawRoundedRectangle(20.f,17.f,480.f,78.f,6.f,1.4f);
 // Instrument modules.
 const auto module=[&](float x,float y,float w,float h,const juce::String& title){
   g.setColour(juce::Colour(0xff0b150c));g.fillRoundedRectangle(x,y,w,h,9.f);
   g.setColour(lime.withAlpha(.44f));g.drawRoundedRectangle(x,y,w,h,9.f,1.1f);
   g.setColour(glow);g.setFont(juce::Font(juce::FontOptions(14.f,juce::Font::bold)));
   g.drawText(title,(int)x+14,(int)y+9,(int)w-26,20,juce::Justification::left);
   g.setColour(lime.withAlpha(.30f));g.drawLine(x+12,y+35,x+w-12,y+35,1.f);
 };
 module(22,140,294,192,"01   OSCILLATORS");
 module(329,140,422,192,"02   WAVEFORM DISPLAY");
 module(764,140,294,192,"03   LFO CONTROLS");
 g.setColour(juce::Colour(0xffa1df8b));g.setFont(11.f);
 g.drawText("OSC 1  //  MORPH  +  FM",39,177,255,16,juce::Justification::left);
 g.drawText("OSC 2  //  DETUNE  +  SUB",39,231,255,16,juce::Justification::left);
 for(int j=0;j<2;++j){
   auto r=juce::Rectangle<float>(39.f,197.f+j*53.f,260.f,38.f);
   g.setColour(juce::Colour(0xff101d10));g.fillRoundedRectangle(r,5.f);
   juce::Path wave;
   for(int i=0;i<240;++i){
     float t=(float)i/240.f;
     float val=j==0?std::sin(t*juce::MathConstants<float>::twoPi*5.f):
       2.f*(t*5.f-std::floor(t*5.f))-1.f;
     const float xx=r.getX()+10.f+(float)i,yy=r.getCentreY()-val*13.f;
     if(i==0)wave.startNewSubPath(xx,yy);else wave.lineTo(xx,yy);
   }
   g.setColour(glow);g.strokePath(wave,juce::PathStrokeType(1.8f));
 }
 // The oscilloscope is a visual preview driven by morph/FM/movement parameters.
 const auto scope=juce::Rectangle<float>(345.f,182.f,390.f,135.f);
 g.setColour(juce::Colour(0xff020802));g.fillRoundedRectangle(scope,6.f);
 g.setColour(lime.withAlpha(.13f));
 for(int i=1;i<10;++i){float x=scope.getX()+scope.getWidth()*i/10.f;g.drawVerticalLine((int)x,scope.getY(),scope.getBottom());}
 for(int i=1;i<4;++i){float y=scope.getY()+scope.getHeight()*i/4.f;g.drawHorizontalLine((int)y,scope.getX(),scope.getRight());}
 const float morph=processor.state.getRawParameterValue("shape")->load();
 const float fm=processor.state.getRawParameterValue("fm")->load();
 const float move=processor.state.getRawParameterValue("movement")->load();
 juce::Path waveform;
 for(int i=0;i<376;++i){
   float t=(float)i/376.f, p=t*juce::MathConstants<float>::twoPi*8.f;
   float value=std::sin(p+fm*2.f*std::sin(p*3.2f))*(.65f+.35f*std::sin(morph+t*12.f*move));
   float x=scope.getX()+7.f+(float)i,y=scope.getCentreY()-value*48.f;
   if(i==0)waveform.startNewSubPath(x,y);else waveform.lineTo(x,y);
 }
 g.setColour(lime.withAlpha(.18f));g.strokePath(waveform,juce::PathStrokeType(8.f));
 g.setColour(lime.withAlpha(.5f));g.strokePath(waveform,juce::PathStrokeType(4.f));
 g.setColour(glow);g.strokePath(waveform,juce::PathStrokeType(1.8f));
 g.setColour(glow.withAlpha(.7f));g.setFont(10.f);
 g.drawText("MORPHING WAVEFORM  //  LIVE PARAMETERS",353,316,345,13,juce::Justification::left);
 g.setColour(lime.withAlpha(.2f));g.drawRoundedRectangle(780.f,188.f,264.f,127.f,6.f,1.f);
 juce::Path lfo;
 for(int i=0;i<240;++i){
  float t=(float)i/240.f,y=std::sin(t*juce::MathConstants<float>::twoPi*2.f);
  if(i==0)lfo.startNewSubPath(790.f,292.f-y*13.f);else lfo.lineTo(790.f+i,292.f-y*13.f);
 }
 g.setColour(glow.withAlpha(.5f));g.strokePath(lfo,juce::PathStrokeType(1.5f));
 const char* titles[]={"SOUND DESIGN  /  OSCILLATORS & DISTORTION",
                      "MOVEMENT  /  FILTER & ENVELOPES",
                      "FINAL SHAPE  /  MIX & OUTPUT"};
 for(int row=0;row<3;++row){
   float y=352.f+row*134.f;
   g.setColour(juce::Colour(0xff0b150c));g.fillRoundedRectangle(22.f,y,1036.f,125.f,9.f);
   g.setColour(lime.withAlpha(.36f));g.drawRoundedRectangle(22.f,y,1036.f,125.f,9.f,1.2f);
   g.setColour(glow);g.setFont(juce::Font(juce::FontOptions(13.f,juce::Font::bold)));
   g.drawText(titles[row],34,(int)y+5,610,19,juce::Justification::left);
   g.setColour(lime.withAlpha(.11f));
   for(int col=1;col<6;++col)g.drawVerticalLine(22+col*172,y+28,y+121);
 }
 g.setColour(lime.withAlpha(.36f));g.drawLine(22.f,765.f,1058.f,765.f,2.f);
 g.setColour(juce::Colour(0xffa7d59a));g.setFont(juce::Font(juce::FontOptions(11.f,juce::Font::bold)));
 g.drawText("SIK RAGE PRODUCTIONS  //  BASS DESIGN ENGINE",26,770,700,20,juce::Justification::left);
 g.setColour(lime);g.drawText("VST3  /  4X OVERSAMPLING",806,770,242,20,juce::Justification::centredRight);
}
void RageForgeEditor::resized(){
 // Match the scalable painted reference. All JUCE child controls stay live.
 const float scale=juce::jmin((float)getWidth()/1080.f,(float)getHeight()/800.f);
 const float ox=((float)getWidth()-1080.f*scale)*.5f;
 const float oy=((float)getHeight()-800.f*scale)*.5f;
 const auto scaled=[=](int x,int y,int w,int h){
   return juce::Rectangle<int>(
     juce::roundToInt(ox+x*scale),juce::roundToInt(oy+y*scale),
     juce::jmax(1,juce::roundToInt(w*scale)),juce::jmax(1,juce::roundToInt(h*scale)));
 };
 brand.setBounds(scaled(26,14,620,69));
 tagline.setFont(juce::Font(juce::FontOptions(juce::jmax(8.f,13.f*scale))));
 tagline.setBounds(scaled(39,81,680,20));
 preset.setBounds(scaled(787,25,257,38));
 subtitle.setFont(juce::Font(juce::FontOptions(juce::jmax(8.f,12.f*scale))));
 subtitle.setBounds(scaled(765,70,292,23));
 for(size_t i=0;i<knobs.size();++i){
   int row=static_cast<int>(i/6),col=static_cast<int>(i%6);
   int x=25+col*172,y=376+row*134;
   knobs[i]->label.setFont(juce::Font(juce::FontOptions(juce::jmax(8.f,11.f*scale),juce::Font::bold)));
   knobs[i]->label.setBounds(scaled(x,y,165,19));
   knobs[i]->slider.setBounds(scaled(x+20,y+17,128,91));
 }
 lfoDivisionLabel.setFont(juce::Font(juce::FontOptions(juce::jmax(8.f,11.f*scale),juce::Font::bold)));
 lfoShapeLabel.setFont(juce::Font(juce::FontOptions(juce::jmax(8.f,11.f*scale),juce::Font::bold)));
 lfoDivisionLabel.setBounds(scaled(781,201,120,16));
 lfoDivision.setBounds(scaled(782,217,120,28));
 lfoShapeLabel.setBounds(scaled(911,201,123,16));
 lfoShape.setBounds(scaled(910,217,125,28));
 mono.setBounds(scaled(820,299,206,24));
}
