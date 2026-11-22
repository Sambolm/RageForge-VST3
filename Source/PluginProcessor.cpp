#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>

namespace {
void add(juce::AudioProcessorValueTreeState::ParameterLayout& l,const char* id,const char* name,float mn,float mx,float init,float skew=1.f){
 auto range=juce::NormalisableRange<float>(mn,mx);range.setSkewForCentre(skew==1.f?(mn+mx)*.5f:skew);
 // APVTS SliderAttachment uses AudioParameterFloat::getText for display.
 const auto attrs=juce::AudioParameterFloatAttributes()
   .withStringFromValueFunction([](float value,int){return juce::String(value,2);})
   .withValueFromStringFunction([](const juce::String& value){return value.getFloatValue();});
 l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(id,1),name,range,init,attrs));
}
}
juce::AudioProcessorValueTreeState::ParameterLayout RageForgeProcessor::createLayout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 add(l,"shape","Wave Morph",0,1,.55);
 add(l,"fm","FM Aggression",0,1,.30);
 add(l,"detune","Detune",0,1,.20);
 add(l,"sub","Sub Layer",0,1,.50);
 add(l,"bassBody","Bass Body",0,1,0);
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
 add(l,"headroom","Headroom (dB)",0,18,3);
 add(l,"waveBlend","Drawn Wave Blend",0,1,0);
 add(l,"waveWarp","Drawn Wave Warp",0,1,0);
 add(l,"growl","Drop Growl",0,1,0);
 add(l,"screech","Drop Screech",0,1,0);
 add(l,"servo","Drop Servo",0,1,0);
 add(l,"punch","Drop Punch",0,1,0);
 add(l,"fmRatio","FM Harmonics",0,1,0);
 add(l,"pitchDive","Laser Dive",0,1,0);
 add(l,"vowelSweep","Vowel Sweep",0,1,0);
 add(l,"midCrunch","Mid Crunch",0,1,0);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("rhythmGrid",1),"Rhythm Grid",juce::StringArray{"1/8","1/16","1/32","1/16 TRIPLET"},1));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("rhythmDensity",1),"Rhythm Density",juce::StringArray{"Sparse","Balanced","Busy"},1));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("rhythmEnabled",1),"Rhythm On",false));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("freshRetrigger",1),"Reset Each Note",true));
 l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("mono",1),"Mono/Legato",true));
 return l;
}
RageForgeProcessor::RageForgeProcessor():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"RAGEFORGE",createLayout()){
 for(int i=0;i<rage::Engine::waveSize;++i)userWave[(size_t)i].store(std::sin(2.f*rage::pi*(float)i/rage::Engine::waveSize));
 const auto startPattern=rage::RhythmPattern::generate(0,0x7f119ab4u,1,1);
 for(int i=0;i<rage::RhythmPattern::steps;++i)rhythmSteps[(size_t)i].store(startPattern.hits[(size_t)i]);
}
bool RageForgeProcessor::isBusesLayoutSupported(const BusesLayout& b) const {return b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void RageForgeProcessor::prepareToPlay(double s,int){
 engine.prepare(static_cast<float>(s));
 rhythmPlayer.reset();rhythmStepDisplay.store(0);rhythmTone=0;wasRhythmEnabled=false;
 wavePlaying.store(false,std::memory_order_relaxed);
 wavePhase.store(0.f,std::memory_order_relaxed);
 waveScopeProgress.store(0.f,std::memory_order_relaxed);
}
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
 p.stereo=val("stereo");p.mix=val("mix");p.glide=val("glide");p.level=val("level");
 p.headroomDb=val("headroom");p.mono=val("mono")>.5f;
 p.waveBlend=val("waveBlend");p.waveWarp=val("waveWarp");p.bassBody=val("bassBody");
 p.growl=val("growl");p.screech=val("screech");p.servo=val("servo");p.punch=val("punch");
 p.fmRatio=val("fmRatio");p.pitchDive=val("pitchDive");
 p.vowelSweep=val("vowelSweep");p.midCrunch=val("midCrunch");
 p.freshRetrigger=val("freshRetrigger")>.5f;
}
void RageForgeProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi){
 juce::ScopedNoDenormals denormals;
 audio.clear();syncParameters();
 for(int k=0;k<rage::Engine::waveSize;++k)engine.setWavePoint(k,userWave[(size_t)k].load(std::memory_order_relaxed));
 if(auto* play=getPlayHead()){
  if(auto pos=play->getPosition())if(auto bpm=pos->getBpm())engine.setTempo(static_cast<float>(*bpm));
 }
 const bool rhythmEnabled=state.getRawParameterValue("rhythmEnabled")->load()>.5f;
 if(!rhythmEnabled&&wasRhythmEnabled){
   if(rhythmPlayer.held||rhythmPlayer.sounding)engine.allNotesOff();
   rhythmPlayer.reset();rhythmTone=0;
 }
 wasRhythmEnabled=rhythmEnabled;
 rage::RhythmPattern currentRhythm;
 const int rhythmGrid=juce::jlimit(0,3,(int)state.getRawParameterValue("rhythmGrid")->load());
 if(rhythmEnabled)
  for(int n=0;n<rage::RhythmPattern::steps;++n)
   currentRhythm.hits[(size_t)n]=rhythmSteps[(size_t)n].load(std::memory_order_relaxed);
 const rage::Params baseParams=engine.p;
 const int total=audio.getNumSamples();
 auto it=midi.cbegin();const auto end=midi.cend();
 for(int i=0;i<total;++i){
  while(it!=end&&(*it).samplePosition<=i){
   const auto msg=(*it).getMessage();
   if(rhythmEnabled){
    if(msg.isNoteOn()){
      // The host supplies the pitch. Rhythm generation never transposes it.
      rhythmLastNote.store(msg.getNoteNumber(),std::memory_order_relaxed);
      if(rhythmPlayer.held||rhythmPlayer.sounding)engine.allNotesOff();
      rhythmPlayer.start(msg.getNoteNumber(),msg.getFloatVelocity());
    }else if(msg.isNoteOff()){
      if(msg.getNoteNumber()==rhythmPlayer.note){
       rhythmPlayer.stop(msg.getNoteNumber());engine.noteOff(msg.getNoteNumber());
      }
    }else if(msg.isAllNotesOff()||msg.isAllSoundOff()){
      rhythmPlayer.reset();engine.allNotesOff();
    }
   }else{
    if(msg.isNoteOn()){
      rhythmLastNote.store(msg.getNoteNumber(),std::memory_order_relaxed);
      engine.noteOn(msg.getNoteNumber(),msg.getFloatVelocity());
    }else if(msg.isNoteOff())engine.noteOff(msg.getNoteNumber());
    else if(msg.isAllNotesOff()||msg.isAllSoundOff())engine.allNotesOff();
   }
   ++it;
  }
  engine.p=baseParams;
  if(rhythmEnabled){
   const auto ev=rhythmPlayer.tick(engine.sr,engine.beatBpm,currentRhythm,rhythmGrid);
   rhythmStepDisplay.store(rhythmPlayer.step,std::memory_order_relaxed);
   if(ev.off)engine.noteOff(rhythmPlayer.note);
   if(ev.on){
    rhythmTone=ev.tone;
    engine.p.glide=0.f;
    engine.p.freshRetrigger=true;
    engine.noteOn(rhythmPlayer.note,ev.velocity);
   }
   // Sound palette changes by accent while MIDI pitch remains fixed.
   const float character=((float)rhythmTone-2.5f)/2.5f;
   engine.p.vowel=rage::clamp(baseParams.vowel+character*.23f,0.f,1.f);
   engine.p.fm=rage::clamp(baseParams.fm+character*.14f,0.f,1.f);
   engine.p.movement=rage::clamp(baseParams.movement*(.86f+.15f*rhythmTone),0.f,1.f);
   engine.p.vowelSweep=rage::clamp(baseParams.vowelSweep+character*.15f,0.f,1.f);
   engine.p.release=std::min(baseParams.release,.028f);
   engine.p.freshRetrigger=true;
   engine.p.glide=0.f;
  }
  const auto out=engine.tick();
  audio.setSample(0,i,out[0]);audio.setSample(1,i,out[1]);
 }
 // One atomic snapshot per block: no locks and no GUI work on the audio thread.
 // Use the most audible voice in poly mode, and the mono voice in mono mode.
 const rage::Voice* displayed=nullptr;
 for(const auto &voice:engine.voices)
   if(voice.stage!=0 && (!displayed || voice.env>displayed->env))
     displayed=&voice;
 if(displayed){
   wavePhase.store(displayed->phase1,std::memory_order_relaxed);
   waveScopeProgress.store(((float)displayed->scopeCycle+displayed->phase1)/8.f,std::memory_order_relaxed);
   wavePlaying.store(true,std::memory_order_relaxed);
 }else{
   wavePlaying.store(false,std::memory_order_relaxed);
 }
}
void RageForgeProcessor::getStateInformation(juce::MemoryBlock& dest){
 auto snapshot=state.copyState();
 juce::StringArray points;
 for(int k=0;k<rage::Engine::waveSize;++k)
   points.add(juce::String(userWave[(size_t)k].load(std::memory_order_relaxed),7));
 snapshot.setProperty("userWave64",points.joinIntoString(","),nullptr);
 juce::StringArray rhythmList;
 for(int i=0;i<rage::RhythmPattern::steps;++i)rhythmList.add(juce::String((int)rhythmSteps[(size_t)i].load()));
 snapshot.setProperty("rhythm32",rhythmList.joinIntoString(","),nullptr);
 auto xml=snapshot.createXml();copyXmlToBinary(*xml,dest);
}
void RageForgeProcessor::setStateInformation(const void* data,int bytes){
 std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,bytes));
 if(xml && xml->hasTagName(state.state.getType())){
   auto restored=juce::ValueTree::fromXml(*xml);
   const auto list=juce::StringArray::fromTokens(restored.getProperty("userWave64").toString(),",","");
   if(list.size()==rage::Engine::waveSize){
     for(int k=0;k<rage::Engine::waveSize;++k)
       setWavePoint(k,list[k].getFloatValue());
   }
   restored.removeProperty("userWave64",nullptr);
   const auto rhythmSaved=juce::StringArray::fromTokens(restored.getProperty("rhythm32").toString(),",","");
   if(rhythmSaved.size()==rage::RhythmPattern::steps)
    for(int i=0;i<rage::RhythmPattern::steps;++i)
     rhythmSteps[(size_t)i].store((uint8_t)juce::jlimit(0,4,rhythmSaved[i].getIntValue()));
   else if(rhythmSaved.size()==32) // legacy 32-step projects
    for(int i=0;i<32;++i)
     rhythmSteps[(size_t)i].store((uint8_t)juce::jlimit(0,4,rhythmSaved[i].getIntValue()));
   restored.removeProperty("rhythm32",nullptr);
   state.replaceState(restored);
 }
}
void RageForgeProcessor::setWavePoint(int index,float value){
 if(index>=0&&index<rage::Engine::waveSize)userWave[(size_t)index].store(rage::clamp(value,-1.f,1.f),std::memory_order_relaxed);
}
float RageForgeProcessor::getWavePoint(int index) const{
 return index>=0&&index<rage::Engine::waveSize?userWave[(size_t)index].load(std::memory_order_relaxed):0.f;
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
  {"waveBlend",0.f},{"waveWarp",0.f},{"freshRetrigger",1.f},
  {"growl",0.f},{"screech",0.f},{"servo",0.f},{"punch",0.f},{"bassBody",0.f},
  {"fmRatio",0.f},{"pitchDive",0.f},{"vowelSweep",0.f},{"midCrunch",0.f},
  {"lfoDivision", static_cast<float>(std::array<int,8>{3,3,2,5,1,4,1,5}[static_cast<size_t>(juce::jlimit(0,7,index))])},
  {"lfoShape", static_cast<float>(std::array<int,8>{4,0,2,1,4,1,3,0}[static_cast<size_t>(juce::jlimit(0,7,index))])}
 };
 for(auto [id,value]:values){auto* par=state.getParameter(id);par->setValueNotifyingHost(par->convertTo0to1(value));}
}
void RageForgeProcessor::selectDropPreset(int index){
 // The macro patches are quick starting points; all parameters remain automatable.
 struct DropPatch{
  float growl,screech,servo,punch,shape,fm,sub,cutoff,resonance,vowel,movement,drive,
        attack,decay,sustain,release,level,headroom;
  int division,shapeLfo;
 };
 static const std::array<DropPatch,5> patches{{
  {.92f,.18f,.20f,.55f,.62f,.68f,.75f,1900.f,.72f,.84f,.95f,.78f,.004f,.18f,.66f,.13f,.46f,6.f,2,2},  // Beast Growl
  {.20f,.97f,.37f,.58f,.91f,.97f,.30f,6800.f,.28f,.20f,.74f,.91f,.003f,.11f,.44f,.15f,.34f,8.f,1,4}, // Razor Screech
  {.38f,.48f,.98f,.73f,.69f,.77f,.60f,3300.f,.50f,.63f,.92f,.77f,.004f,.13f,.50f,.12f,.41f,7.f,2,3}, // Servo Machine
  {.59f,.39f,.24f,1.00f,.77f,.72f,.86f,2500.f,.41f,.70f,.87f,.86f,.003f,.08f,.10f,.09f,.50f,7.f,1,4}, // Impact Chop
  {.82f,.76f,.63f,.68f,.83f,.86f,.69f,3900.f,.58f,.74f,.99f,.88f,.004f,.20f,.70f,.20f,.38f,9.f,3,2}  // Mutant Hybrid
 }};
 const auto& p=patches[static_cast<size_t>(juce::jlimit(0,4,index))];
 const std::pair<const char*,float> values[]{
  {"growl",p.growl},{"screech",p.screech},{"servo",p.servo},{"punch",p.punch},
  {"shape",p.shape},{"fm",p.fm},{"sub",p.sub},{"bassBody",.60f},{"cutoff",p.cutoff},
  {"resonance",p.resonance},{"vowel",p.vowel},{"movement",p.movement},
  {"drive",p.drive},{"attack",p.attack},{"decay",p.decay},
  {"sustain",p.sustain},{"release",p.release},{"level",p.level},
  {"headroom",p.headroom},{"mono",1.f},{"freshRetrigger",1.f},{"mix",.97f},{"stereo",.12f},
  {"waveBlend",0.f},{"waveWarp",0.f},{"glide",0.f},
  {"fmRatio",p.screech*.55f},{"pitchDive",p.servo*.62f},
  {"vowelSweep",p.growl*.76f},{"midCrunch",(.35f+.48f*p.screech)},
  {"lfoDivision",(float)p.division},{"lfoShape",(float)p.shapeLfo}
 };
 for(const auto& pair:values)
  if(auto* param=state.getParameter(pair.first))
   param->setValueNotifyingHost(param->convertTo0to1(pair.second));
}
void RageForgeProcessor::generateDropVariation(int family){
 // Sound design, not a random value dump: preserve the five bass archetypes.
 // Different seeds create controlled harmonic, rhythmic, and transient variation.
 family=juce::jlimit(0,4,family);
 selectDropPreset(family);
 uint32_t seed=0x9e3779b9u+((++variationCounter)*747796405u)+(uint32_t)family*2891336453u;
 auto random=[&seed](){
   seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
   return (float)(seed&0x00ffffffu)/16777215.f;
 };
 auto between=[&](float low,float high){return low+(high-low)*random();};
 auto set=[this](const char* id,float value){
   if(auto* param=state.getParameter(id))
    param->setValueNotifyingHost(param->convertTo0to1(value));
 };
 // Choose FM, formant and rhythm as a coherent family rather than independently.
 const int rhythm=(int)(random()*3.999f);
 constexpr int divisions[4]={1,2,3,4};
 constexpr int lfoShapes[4]={1,2,3,4};
 set("lfoDivision",(float)divisions[rhythm]);
 set("lfoShape",(float)lfoShapes[(rhythm+family)%4]);
 set("detune",between(.03f,.19f));
 set("attack",between(.003f,.009f));
 set("glide",0.f);
 set("stereo",between(.05f,.18f));
 set("headroom",between(6.f,10.f));
 set("mix",between(.83f,.99f));
 if(family==0){ // organic throat growls, lower center of gravity
   set("growl",between(.76f,.99f));set("screech",between(.06f,.30f));
   set("servo",between(.08f,.32f));set("punch",between(.38f,.73f));
   set("fm",between(.39f,.72f));set("cutoff",between(1100.f,3000.f));
   set("vowel",between(.67f,.99f));set("movement",between(.70f,.99f));
   set("resonance",between(.42f,.78f));set("drive",between(.52f,.83f));
   set("sub",between(.65f,.90f));set("release",between(.10f,.24f));
 }else if(family==1){ // bright ripping FM screams
   set("growl",between(.12f,.37f));set("screech",between(.76f,1.f));
   set("servo",between(.17f,.53f));set("punch",between(.44f,.81f));
   set("fm",between(.75f,1.f));set("cutoff",between(4700.f,10300.f));
   set("vowel",between(.09f,.43f));set("movement",between(.48f,.88f));
   set("resonance",between(.12f,.40f));set("drive",between(.71f,.95f));
   set("sub",between(.27f,.57f));set("release",between(.08f,.19f));
 }else if(family==2){ // short pitch sweeps and cyber-mechanical motion
   set("growl",between(.14f,.50f));set("screech",between(.30f,.67f));
   set("servo",between(.81f,1.f));set("punch",between(.53f,.91f));
   set("fm",between(.46f,.87f));set("cutoff",between(1900.f,5500.f));
   set("vowel",between(.29f,.70f));set("movement",between(.73f,1.f));
   set("resonance",between(.31f,.65f));set("drive",between(.58f,.87f));
   set("sub",between(.50f,.77f));set("release",between(.07f,.16f));
 }else if(family==3){ // clipped, gated riddim attacks
   set("growl",between(.40f,.80f));set("screech",between(.21f,.53f));
   set("servo",between(.08f,.44f));set("punch",between(.87f,1.f));
   set("fm",between(.43f,.80f));set("cutoff",between(1700.f,4600.f));
   set("vowel",between(.51f,.84f));set("movement",between(.78f,1.f));
   set("resonance",between(.34f,.70f));set("drive",between(.63f,.89f));
   set("sub",between(.64f,.91f));set("decay",between(.05f,.12f));
   set("sustain",between(.05f,.25f));set("release",between(.05f,.11f));
 }else{ // hybrid, conflicting but musical textures
   set("growl",between(.64f,.96f));set("screech",between(.55f,.91f));
   set("servo",between(.42f,.81f));set("punch",between(.44f,.79f));
   set("fm",between(.63f,.94f));set("cutoff",between(2300.f,6900.f));
   set("vowel",between(.53f,.91f));set("movement",between(.73f,1.f));
   set("resonance",between(.39f,.68f));set("drive",between(.69f,.95f));
   set("sub",between(.54f,.82f));set("release",between(.10f,.22f));
 }
 set("shape",between(family==1?.68f:.33f,.96f));
 set("level",between(.36f,.51f));
 set("bassBody",between(family==1?.35f:.48f,family==1?.64f:.86f));
 // Harmonic, periodic custom waves with a controlled number of partials.
 // Avoid sample-import chops or high-noise waveforms.
 const float h2=between(.15f,.46f);
 const float h3=between(.06f,.35f);
 const float h5=between(.03f,.19f);
 const float h7=between(.0f,.10f);
 const float phase=between(.0f,6.2831853f);
 const float sign=family==1?-1.f:1.f;
 float wave[rage::Engine::waveSize];
 float maxAbs=.001f;
 for(int i=0;i<rage::Engine::waveSize;++i){
   const float t=2.f*rage::pi*(float)i/rage::Engine::waveSize;
   wave[i]=std::sin(t)+sign*h2*std::sin(2.f*t+phase)
          +h3*std::sin(3.f*t-phase*.35f)+h5*std::sin(5.f*t+.3f*phase)
          +h7*std::sin(7.f*t);
   maxAbs=std::max(maxAbs,std::abs(wave[i]));
 }
 for(int i=0;i<rage::Engine::waveSize;++i)setWavePoint(i,wave[i]/maxAbs);
 set("waveBlend",between(.56f,.91f));
 set("waveWarp",between(.08f,.62f));
 set("fmRatio",between(family==1?.48f:.06f,family==1?.96f:.69f));
 set("pitchDive",between(family==2?.45f:.08f,family==2?.97f:.67f));
 set("vowelSweep",between(family==0?.62f:.13f,family==0?.97f:.84f));
 set("midCrunch",between(family==3?.57f:.28f,family==3?.95f:.88f));
}
void RageForgeProcessor::generateRhythm(int family){
 const uint32_t seed=0x352bfa91u+((++rhythmCounter)*747796405u)
      +(uint32_t)juce::jlimit(0,4,family)*2891336453u;
 const int grid=juce::jlimit(0,3,(int)state.getRawParameterValue("rhythmGrid")->load());
 const int density=juce::jlimit(0,2,(int)state.getRawParameterValue("rhythmDensity")->load());
 const auto pattern=rage::RhythmPattern::generate(family,seed,grid,density);
 for(int i=0;i<rage::RhythmPattern::steps;++i)
   rhythmSteps[(size_t)i].store(pattern.hits[(size_t)i],std::memory_order_relaxed);
}
int RageForgeProcessor::getRhythmStep(int i) const{
 return i>=0&&i<rage::RhythmPattern::steps
   ?(int)rhythmSteps[(size_t)i].load(std::memory_order_relaxed):0;
}
juce::AudioProcessorEditor* RageForgeProcessor::createEditor(){return new RageForgeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new RageForgeProcessor();}
