#include "../Source/RageDSP.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
using namespace rage;
struct Metrics{double energy=0,peak=0,sum=0;int samples=0;};
Metrics render(Engine &e,int n){Metrics m;for(int i=0;i<n;++i){auto x=e.tick();for(auto s:x){assert(std::isfinite(s));m.peak=std::max(m.peak,double(std::abs(s)));m.energy+=s*s;m.sum+=s;}m.samples++;}return m;}
int main(){
 Engine e;e.prepare(48000);e.noteOn(41,1);auto a=render(e,48000);assert(a.energy>10 && a.peak<1.01);e.noteOff(41);render(e,48000);auto tail=render(e,48000);assert(tail.energy<.01);
 Engine f;f.prepare(48000);f.p.shape=.05;f.p.fm=.05;f.noteOn(41,1);std::vector<float> dry(24000);for(auto &s:dry)s=f.tick()[0];
 f.prepare(48000);f.p.shape=.95;f.p.fm=.95;f.p.vowel=.9;f.p.drive=.90;f.noteOn(41,1);double diff=0;for(float x:dry){float y=f.tick()[0];diff+=(x-y)*(x-y);}assert(diff>1.0);
 Engine p;p.prepare(48000);p.p.mono=false;p.noteOn(48,1);p.noteOn(55,1);auto poly=render(p,24000);assert(poly.energy>1);
 Engine low;low.prepare(48000);low.p.sub=1;low.p.drive=.9;low.noteOn(29,.95);auto b=render(low,24000);assert(b.energy>1&&b.peak<1.01);
 assert(std::abs(lfoBeatsPerCycle(3)-1.f)<1e-6f);
 assert(std::abs(lfoBeatsPerCycle(5)-4.f)<1e-6f);
 assert(lfoValue(.25f,2) < lfoValue(.75f,2));
 assert(lfoValue(.25f,3) > lfoValue(.75f,3));
 assert(lfoValue(.25f,4) != lfoValue(.75f,4));
 Engine modA;modA.prepare(48000);modA.p.movement=1;modA.p.lfoDivision=2;modA.p.lfoShape=0;modA.noteOn(41,1);
 std::vector<float> sine(24000);for(auto &s:sine)s=modA.tick()[0];
 Engine modB;modB.prepare(48000);modB.p.movement=1;modB.p.lfoDivision=2;modB.p.lfoShape=4;modB.noteOn(41,1);
 double shapeDiff=0;for(auto s:sine){const float d=s-modB.tick()[0];shapeDiff+=d*d;}
 assert(shapeDiff>.05);
 // Guard against the click previously caused by resetting an active mono voice.
 Engine loop;loop.prepare(48000);loop.p.mono=true;loop.p.attack=.001f;loop.p.headroomDb=0.f;
 loop.noteOn(41,1.f);
 float previous=0.f,maximum=0.f;
 for(int i=0;i<9600;++i){auto x=loop.tick();previous=x[0];maximum=std::max(maximum,std::abs(previous));}
 loop.noteOn(41,1.f);
 const float first=loop.tick()[0];
 assert(std::abs(first-previous)<.15f);
 assert(maximum<=.892f);
 // User headroom must actually attenuate the output, with clipping ceiling intact.
 Engine loud;loud.prepare(48000);loud.p.headroomDb=0.f;loud.noteOn(41,1.f);
 const auto loudMetrics=render(loud,12000);
 Engine quiet;quiet.prepare(48000);quiet.p.headroomDb=12.f;quiet.noteOn(41,1.f);
 const auto quietMetrics=render(quiet,12000);
 assert(quietMetrics.energy<loudMetrics.energy*.20);
 assert(loudMetrics.peak<=.892f && quietMetrics.peak<=.892f);
 // The user-drawn wavetable must change audible synthesis, not just the UI.
 Engine base;base.prepare(48000);base.p.waveBlend=1.f;base.noteOn(41,1.f);
 std::vector<float> reference(24000);
 for(auto &v:reference)v=base.tick()[0];
 Engine drawn;drawn.prepare(48000);drawn.p.waveBlend=1.f;drawn.p.waveWarp=.60f;
 for(int i=0;i<Engine::waveSize;++i)drawn.setWavePoint(i,i<Engine::waveSize/2?1.f:-1.f);
 drawn.noteOn(41,1.f);
 double userWaveDiff=0;for(auto v:reference){const double d=v-drawn.tick()[0];userWaveDiff+=d*d;}
 assert(userWaveDiff>0.5);
 Engine off;off.prepare(48000);off.p.waveBlend=0;off.noteOn(41,1.f);
 Engine offDraw;offDraw.prepare(48000);offDraw.p.waveBlend=0;
 for(int i=0;i<Engine::waveSize;++i)offDraw.setWavePoint(i,i%2?1.f:-1.f);
 offDraw.noteOn(41,1.f);
 double bypassDiff=0;for(int i=0;i<12000;++i){double d=off.tick()[0]-offDraw.tick()[0];bypassDiff+=d*d;}
 assert(bypassDiff<1e-8);
 // The visual playhead is derived from real oscillator phase and cycle count.
 Engine cursor;cursor.prepare(48000);cursor.noteOn(41,1.f);
 int sawWrapping=0;int previousCycle=0;
 for(int i=0;i<48000;++i){
   cursor.tick();
   const auto& v=cursor.voices[0];
   assert(v.phase1>=0.f&&v.phase1<1.f);
   assert(v.scopeCycle>=0&&v.scopeCycle<8);
   if(v.scopeCycle!=previousCycle)++sawWrapping;
   previousCycle=v.scopeCycle;
 }
 assert(sawWrapping>30);
 // Regression: released notes must not legato-glide through an old release tail.
 Engine transition;transition.prepare(48000);transition.p.mono=true;transition.p.glide=.08f;
 transition.p.release=.22f;
 transition.noteOn(60,1.f);render(transition,3600);
 transition.noteOff(60);render(transition,650);
 assert(transition.voices[0].stage==4 && !transition.voices[0].holding);
 transition.noteOn(61,1.f);
 assert(transition.voices[0].note==61);
 assert(transition.voices[0].holding);
 assert(transition.voices[0].stage==1); // must REATTACK, not continue releasing
 const float envBefore=transition.voices[0].env;
 render(transition,250);
 assert(transition.voices[0].env>envBefore);
 assert(transition.voices[0].stage!=4);
 // When C and C# are genuinely overlapping, legato should preserve the envelope.
 Engine overlap;overlap.prepare(48000);overlap.p.mono=true;overlap.p.glide=.08f;
 overlap.noteOn(60,1.f);render(overlap,2400);
 const float heldEnv=overlap.voices[0].env;
 const int heldStage=overlap.voices[0].stage;
 overlap.noteOn(61,1.f);
 assert(overlap.voices[0].stage==heldStage);
 assert(std::abs(overlap.voices[0].env-heldEnv)<1e-5f);
 // The old note-off must never silence the newer held note.
 overlap.noteOff(60);
 assert(overlap.voices[0].holding && overlap.voices[0].note==61);
 // Global mono transition regression: all keyboard intervals and both glide settings.
 // Different note changes must never inherit the old RELEASE envelope.
 for(float glideValue:{0.f,.08f}){
  for(int baseNote:{36,48,60,72}){
   for(int interval:{-12,-7,-2,-1,0,1,2,7,12}){
    const int nextNote=baseNote+interval;
    if(nextNote<0||nextNote>127)continue;
    Engine change;change.prepare(48000);
    change.p.mono=true;change.p.glide=glideValue;change.p.attack=.003f;change.p.release=.2f;
    change.noteOn(baseNote,.8f);
    render(change,3600);
    change.noteOff(baseNote);
    render(change,480);
    assert(change.voices[0].stage==4 && !change.voices[0].holding);
    const float before=change.tick()[0];
    change.noteOn(nextNote,.8f);
    const auto& voice=change.voices[0];
    assert(voice.note==nextNote && voice.holding && voice.stage==1);
    const float after=change.tick()[0];
    assert(std::isfinite(after) && std::abs(after-before)<.25f);
    // Must sustain, not give a second delayed attack/release.
    render(change,2000);
    assert(change.voices[0].holding && change.voices[0].stage!=4);
    change.noteOff(nextNote);
    assert(!change.voices[0].holding);
   }
  }
 }
 // Overlapping transitions: no envelope restart, even through large pitch jumps.
 for(int interval:{-12,-7,-1,1,7,12}){
  Engine legato;legato.prepare(48000);legato.p.mono=true;legato.p.glide=.06f;
  legato.noteOn(60,1.f);render(legato,3600);
  const float beforeEnv=legato.voices[0].env;
  const int beforeStage=legato.voices[0].stage;
  legato.noteOn(60+interval,1.f);
  assert(legato.voices[0].stage==beforeStage);
  assert(std::abs(legato.voices[0].env-beforeEnv)<1e-5f);
  legato.noteOff(60);
  assert(legato.voices[0].holding);
 }
 // Drop Lab must actually transform audio. Zero macros retain legacy engine output.
 const auto bassTest=[](float growl,float screech,float servo,float punch){
   Engine e;e.prepare(48000);e.p.growl=growl;e.p.screech=screech;
   e.p.servo=servo;e.p.punch=punch;e.p.shape=.66f;e.p.fm=.57f;
   e.p.cutoff=2600.f;e.p.movement=.82f;e.p.vowel=.73f;
   e.noteOn(36,1.f);
   std::vector<float> out(12000);
   for(auto &v:out){v=e.tick()[0];assert(std::isfinite(v)&&std::abs(v)<=.892f);}
   return out;
 };
 const auto legacy=bassTest(0,0,0,0);
 const std::array<std::array<float,4>,5> macros{{
  {1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},{0.f,0.f,1.f,0.f},
  {0.f,0.f,0.f,1.f},{.8f,.7f,.6f,.5f}
 }};
 std::array<std::vector<float>,5> bassOutputs;
 for(size_t i=0;i<macros.size();++i){
   bassOutputs[i]=bassTest(macros[i][0],macros[i][1],macros[i][2],macros[i][3]);
   double diffBass=0;
   for(size_t j=0;j<legacy.size();++j){
     const double d=(double)bassOutputs[i][j]-(double)legacy[j];
     diffBass+=d*d;
   }
   assert(diffBass>.0001);
 }
 for(size_t i=0;i<bassOutputs.size();++i)
  for(size_t j=i+1;j<bassOutputs.size();++j){
   double diffBass=0;
   for(size_t k=0;k<bassOutputs[i].size();++k){
    const double d=(double)bassOutputs[i][k]-(double)bassOutputs[j][k];
    diffBass+=d*d;
   }
   assert(diffBass>.0001);
  }
 // Reference-inspired bass transformations must each produce distinct samples,
 // remain finite, and respect the output safety ceiling.
 auto signatureRender=[](float fmRatio,float pitchDive,float vowelSweep,float midCrunch){
   Engine synth;synth.prepare(44100);
   synth.p.fm=.74f;synth.p.vowel=.82f;synth.p.movement=.82f;
   synth.p.growl=.58f;synth.p.screech=.42f;synth.p.punch=.48f;
   synth.p.fmRatio=fmRatio;synth.p.pitchDive=pitchDive;
   synth.p.vowelSweep=vowelSweep;synth.p.midCrunch=midCrunch;
   synth.noteOn(33,1.f);
   std::vector<float> samples(10000);
   for(auto &sample:samples){
     sample=synth.tick()[0];
     assert(std::isfinite(sample)&&std::abs(sample)<=.892f);
   }
   return samples;
 };
 const auto signatureBase=signatureRender(0,0,0,0);
 for(const auto &variation:std::array<std::array<float,4>,4>{{
   {{1.f,0.f,0.f,0.f}},{{0.f,1.f,0.f,0.f}},
   {{0.f,0.f,1.f,0.f}},{{0.f,0.f,0.f,1.f}}
 }}){
   auto output=signatureRender(variation[0],variation[1],variation[2],variation[3]);
   double difference=0;
   for(size_t k=0;k<output.size();++k){
     const double d=(double)signatureBase[k]-output[k];difference+=d*d;
   }
   assert(difference>.0001);
 }
 // Fresh retrigger regression: discrete MIDI hits always restart oscillator,
 // FM, sub, LFO, filter states, and note age instead of resuming mid-pattern.
 for(int secondNote:{36,37,43,48,60}){
  Engine resetBass;resetBass.prepare(48000);resetBass.p.mono=true;
  resetBass.p.freshRetrigger=true;resetBass.p.glide=0.f;resetBass.p.movement=.95f;
  resetBass.p.growl=.89f;resetBass.p.fm=.78f;resetBass.p.pitchDive=.8f;
  resetBass.noteOn(36,1.f);render(resetBass,5071);
  resetBass.noteOff(36);render(resetBass,93);
  const auto before=resetBass.tick()[0];
  assert(resetBass.voices[0].phase1>0.f);
  assert(resetBass.voices[0].lfo>0.f);
  resetBass.noteOn(secondNote,1.f);
  const auto& rv=resetBass.voices[0];
  assert(rv.note==secondNote && rv.holding && rv.stage==1);
  assert(rv.env==0.f && rv.phase1==0.f && rv.phase2==0.f);
  assert(rv.phaseFM==0.f && rv.phaseSub==0.f);
  assert(rv.lfo==0.f && rv.noteAge==0.f);
  assert(rv.main.ic1==0.f && rv.form1.ic2==0.f);
  assert(rv.lowBand==0.f && rv.highBand==0.f);
  assert(resetBass.retriggerFadeRemaining>0);
  const auto first=resetBass.tick()[0];
  assert(std::abs(first-before)<.03f);
  render(resetBass,800);
  assert(resetBass.voices[0].note==secondNote);
  assert(resetBass.voices[0].phase1>0.f && resetBass.voices[0].noteAge>0.f);
 }
 // Classic continuation remains selectable for old session aesthetics.
 Engine flowing;flowing.prepare(48000);flowing.p.mono=true;
 flowing.p.freshRetrigger=false;flowing.p.glide=0.f;
 flowing.noteOn(48,1.f);render(flowing,4101);flowing.noteOff(48);render(flowing,100);
 const float oldPhase=flowing.voices[0].phase1,oldLfo=flowing.voices[0].lfo;
 flowing.noteOn(50,1.f);
 assert(std::abs(flowing.voices[0].phase1-oldPhase)<1e-6f);
 assert(std::abs(flowing.voices[0].lfo-oldLfo)<1e-6f);
 // A genuinely overlapping glide remains phase-continuous with Reset Each Note enabled.
 Engine linked;linked.prepare(48000);linked.p.mono=true;
 linked.p.freshRetrigger=true;linked.p.glide=.09f;
 linked.noteOn(48,1.f);render(linked,4101);
 const float linkedPhase=linked.voices[0].phase1,linkedLfo=linked.voices[0].lfo;
 linked.noteOn(49,1.f);
 assert(std::abs(linked.voices[0].phase1-linkedPhase)<1e-6f);
 assert(std::abs(linked.voices[0].lfo-linkedLfo)<1e-6f);
 assert(linked.retriggerFadeRemaining==0);
 // Two-bar signature rhythm generator: tempo-locked, accented, with rests,
 // and no unintended pitch changes between rhythmic events.
 for(int family=0;family<5;++family){
   auto pattern=rage::RhythmPattern::generate(family,0xabcdef01u+(uint32_t)family);
   auto variant=rage::RhythmPattern::generate(family,0xa7196fb4u+(uint32_t)family);
   assert(pattern.hits[0]==4&&pattern.hits[16]==4);
   int rests=0,hits=0,changes=0;
   for(int i=0;i<rage::RhythmPattern::steps;++i){
     const auto v=pattern.hits[(size_t)i];
     assert(v<=4);
     if(v==0)++rests;else ++hits;
     if(v!=variant.hits[(size_t)i])++changes;
   }
   assert(rests>7 && hits>8 && changes>0);
   rage::RhythmPlayer rhythm;
   rhythm.start(41,.95f);
   const float rate=48000.f,bpm=140.f;
   int totalOn=0,totalOff=0;
   for(int i=0;i<(int)(rate*60.f/bpm*8.f);++i){
     auto ev=rhythm.tick(rate,bpm,pattern);
     if(ev.on){
       assert(ev.tone>0&&ev.tone<=4);
       assert(rhythm.note==41); // all hits use the held note
       ++totalOn;
     }
     if(ev.off)++totalOff;
     assert(rhythm.step>=0&&rhythm.step<32);
   }
   assert(totalOn>=hits-1 && totalOn<=hits+1);
   assert(totalOff>=totalOn-1);
   rhythm.stop(41);
   bool accidentallyRestarted=false;
   for(int i=0;i<20000;++i)
     if(rhythm.tick(rate,bpm,pattern).on)accidentallyRestarted=true;
   assert(!accidentallyRestarted);
 }
 // Explicit subdivision regression: two bars remain eight beats long,
 // with playable 1/8, 1/16, 1/32 and 1/16-triplet hits.
 for(int grid=0;grid<4;++grid){
   const int steps=rage::RhythmPattern::stepCount(grid);
   assert(steps==(grid==0?16:grid==1?32:grid==2?64:48));
   for(int density=0;density<3;++density){
     const auto groove=rage::RhythmPattern::generate(3,0x93abcdefu+grid*127u+density*7u,grid,density);
     assert(groove.hits[0]==4&&groove.hits[(size_t)(steps/2)]==4);
     int accents=0;
     for(int k=0;k<steps;++k)if(groove.hits[(size_t)k])++accents;
     assert(accents>=2&&accents<=steps);
     rage::RhythmPlayer player;
     player.start(44,.8f);
     const double rate=48000.0,bpm=120.0;
     const int samples=(int)(rate*60.0/bpm*8.0);
     int on=0;
     for(int k=0;k<samples;++k){
       auto e=player.tick((float)rate,(float)bpm,groove,grid);
       if(e.on){assert(e.tone>=1&&e.tone<=4);assert(player.note==44);++on;}
     }
     assert(on>=accents-1&&on<=accents+1);
     assert(player.step==0||player.step==steps-1); // two-bar wrap
   }
 }
 // Bass Body: fundamental reinforcement must be audible, controllable,
 // non-clipping, and inactive at zero for existing projects.
 for(int bassNote:{29,36,41,48,53}){
  Engine original;original.prepare(48000);
  original.p.sub=.45f;original.p.bassBody=0.f;original.noteOn(bassNote,.9f);
  Engine reinforced;reinforced.prepare(48000);
  reinforced.p.sub=.45f;reinforced.p.bassBody=.85f;reinforced.noteOn(bassNote,.9f);
  double lowDifference=0;
  for(int i=0;i<12000;++i){
   const float a=original.tick()[0],b=reinforced.tick()[0];
   assert(std::isfinite(a)&&std::isfinite(b));
   assert(std::abs(a)<=.892f&&std::abs(b)<=.892f);
   const double d=(double)b-(double)a;
   lowDifference+=d*d;
  }
  assert(lowDifference>.001);
 }
 Engine bodyA;bodyA.prepare(48000);bodyA.noteOn(41,1.f);
 Engine bodyB;bodyB.prepare(48000);bodyB.p.bassBody=0.f;bodyB.noteOn(41,1.f);
 for(int i=0;i<4800;++i)assert(bodyA.tick()==bodyB.tick());
 std::cout<<"PASS: finite stereo synthesis, output ceiling, note-off release, strongly differentiated timbres, polyphony, sub bass. diff="<<diff<<"\n";
}
