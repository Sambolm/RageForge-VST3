#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>

namespace {
void add(juce::AudioProcessorValueTreeState::ParameterLayout& l,const char* id,const char* name,float mn,float mx,float init,float skew=1.f){
 auto range=juce::NormalisableRange<float>(mn,mx);range.setSkewForCentre(skew==1.f?(mn+mx)*.5f:skew);
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(id,1),name,range,init));
}
}
juce::AudioProcessorValueTreeState::ParameterLayout RageForgeProcessor::createLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 add(l,"shape","Wave Morph",0,1,.55);
 add(l,"fm","FM Aggression",0,1,.30);
 add(l,"detune","Detune",0,1,.20);
 add(l,"sub","Sub Layer",0,1,.50);
 add(l,"cutoff","Filter Cutoff",60,16000,2400,1700);
 add(l,"resonance","Resonance",0,.94,.18);
 add(l,"vowel","Vocal Formant",0,1,.45);
 add(l,"movement","Movement",0,1,.55);
 // Keep the old rate parameter so existing saved projects still load.
 add(l,"rate","Legacy LFO Rate",.125,16,2,2);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("lfoDivision",1),"LFO Sync",juce::StringArray{"1/32","1/16","1/8","1/4","1/2","1 BAR","2 BARS"},3));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("lfoShape",1),"LFO Shape",juce::StringArray{"Sine","Triangle","Saw Up","Saw Down","Square"},0));
 add(l,"drive","Drive",0,1,.50);
 add(l,"attack","Attack",.001,1,.005,.08);
 add(l,"decay","Decay",.02,2,.17,.3);
 add(l,"sustain","Sustain",0,1,.65);
 add(l,"release","Release",.02,3,.18,.35);
 add(l,"stereo","Stereo Spread",0,1,.35);
 add(l,"mix","Wet Mix",0,1,.95);
 add(l,"glide","Portamento",0,1,0,.15);
 add(l,"level","Output Level",0,1,.50);
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("mono",1),"Mono/Legato",true));
 return l;
}
RageForgeProcessor::RageForgeProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"RAGEFORGE",createLayout()){}
bool RageForgeProcessor::isBusesLayoutSupported(const BusesLayout& b) const {return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void RageForgeProcessor::prepareToPlay(double s,int){engine.prepare(static_cast<float>(s));}
void RageForgeProcessor::syncParameters(){
 auto val=[this](const char*s){return state.getRawParameterValue(s)->load();};
 auto &p=engine.p;
 p.shape=val("shape");p.fm=val("fm");p.detune=val("detune");p.sub=val("sub");
 p.cutoff=val("cutoff");p.resonance=val("resonance");p.vowel=val("vowel");
 p.movement=val("movement");p.rate=val("rate");
 p.lfoDivision=juce::jlimit(0,6,static_cast<int>(val("lfoDivision")));
 p.lfoShape=juce::jlimit(0,4,static_cast<int>(val("lfoShape")));
 p.drive=val("drive");
 p.attack=val("attack");p.decay=val("decay");p.sustain=val("sustain");p.release=val("release");
 p.stereo=val("stereo");p.mix=val("mix");p.glide=val("glide");p.level=val("level");p.mono=val("mono")>.5f;
}
void RageForgeProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals denormals;
 audio.clear();syncParameters();
 if(auto* play=getPlayHead()){
  if(auto pos=play->getPosition())if(auto bpm=pos->getBpm())engine.setTempo(static_cast<float>(*bpm));
 }
 int i=0;auto it=midi.cbegin();const auto end=midi.cend();
 for(;i<audio.getNumSamples();++i){
  while(it!=end && (*it).samplePosition<=i){
    auto msg=(*it).getMessage();
    if(msg.isNoteOn())engine.noteOn(msg.getNoteNumber(),msg.getFloatVelocity());
    else if(msg.isNoteOff())engine.noteOff(msg.getNoteNumber());
    else if(msg.isAllNotesOff() || msg.isAllSoundOff())engine.allNotesOff();
    ++it;
  }
  auto out=engine.tick();audio.setSample(0,i,out[0]);audio.setSample(1,i,out[1]);
 }
}
void RageForgeProcessor::getStateInformation(juce::MemoryBlock& dest){
 auto xml=state.copyState().createXml();copyXmlToBinary(*xml,dest);
}
void RageForgeProcessor::setStateInformation(const void* data,int bytes){
 std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,bytes));
 if(xml && xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));
}
void RageForgeProcessor::selectPreset(int index){
 // Sound-engine starting points, not mastered presets. Every parameter remains editable.
 struct Patch {float sh,fm,det,sub,cut,res,vow,mov,rate,drive,a,d,s,r,spread,mix,glide,level;bool mono;};
 static const std::array<Patch,8> presets{{
  {.68f,.65f,.10f,.60f,4200,.2f,.6f,.75f,4,.72f,.002f,.09f,.16f,.11f,.10f,.98f,0,.55f,true},  // stab
  {.50f,.76f,.25f,.72f,2650,.54f,.92f,.95f,2,.69f,.004f,.21f,.74f,.33f,.18f,.96f,.02f,.45f,true}, // talk
  {.88f,.92f,.35f,.43f,7100,.22f,.25f,.77f,8,.82f,.001f,.13f,.40f,.18f,.62f,.98f,0,.4f,true},  // metal
  {.38f,.21f,.87f,.84f,1250,.24f,.14f,.28f,.5f,.32f,.015f,.3f,.93f,.52f,.8f,.83f,.07f,.53f,false}, // reese
  {.73f,.52f,.02f,.72f,3100,.68f,.88f,.91f,8,.74f,.001f,.04f,.10f,.075f,.06f,.96f,0,.56f,true}, // riddim
  {.21f,.40f,.34f,.66f,1900,.75f,.98f,.78f,1,.51f,.009f,.45f,.84f,.58f,.50f,.88f,.09f,.45f,true}, // liquid
  {.93f,.85f,.10f,.22f,9700,.08f,.06f,.52f,16,.96f,.001f,.04f,.0f,.07f,.35f,.99f,0,.37f,true}, // laser
  {.10f,.08f,.03f,.99f,850,.09f,.1f,.12f,.25f,.22f,.003f,.18f,.86f,.3f,.05f,.73f,0,.65f,true} // sub
 }};
 const auto &v=presets[static_cast<size_t>(juce::jlimit(0,7,index))];
 const std::pair<const char*,float> values[]{
  {"shape",v.sh},{"fm",v.fm},{"detune",v.det},{"sub",v.sub},{"cutoff",v.cut},{"resonance",v.res},
  {"vowel",v.vow},{"movement",v.mov},{"rate",v.rate},{"drive",v.drive},{"attack",v.a},
  {"decay",v.d},{"sustain",v.s},{"release",v.r},{"stereo",v.spread},{"mix",v.mix},
  {"glide",v.glide},{"level",v.level},{"mono",v.mono?1.f:0.f},
  {"lfoDivision", static_cast<float>(std::array<int,8>{3,3,2,5,1,4,1,5}[static_cast<size_t>(juce::jlimit(0,7,index))])},
  {"lfoShape", static_cast<float>(std::array<int,8>{4,0,2,1,4,1,3,0}[static_cast<size_t>(juce::jlimit(0,7,index))])}
 };
 for(auto [id,value]:values){auto* par=state.getParameter(id);par->setValueNotifyingHost(par->convertTo0to1(value));}
}
juce::AudioProcessorEditor* RageForgeProcessor::createEditor(){return new RageForgeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new RageForgeProcessor();}
