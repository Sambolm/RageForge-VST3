#pragma once
// RageForge original DSP core. No dependencies on JUCE or external audio libraries.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rage {
constexpr float pi = 3.14159265358979323846f;
inline float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
inline float sat(float x){return std::tanh(x);}
inline float fract(float x){return x-std::floor(x);}
inline float polyblep(float t,float dt){if(dt<=0)return 0; if(t<dt){float x=t/dt;return x+x-x*x-1;}if(t>1-dt){float x=(t-1)/dt;return x*x+x+x+1;}return 0;}
inline float osc(float phase,float dt,float shape){
  const float sine=std::sin(2*pi*phase);
  const float saw=(2*phase-1)-polyblep(phase,dt);
  float pulse=(phase<.5f?1.f:-1.f)+polyblep(phase,dt)-polyblep(fract(phase+.5f),dt);
  float tri=2*std::abs(2*phase-1)-1;
  shape=clamp(shape,0,1);
  if(shape<.3333f)return sine+(saw-sine)*shape*3.f;
  if(shape<.6667f)return saw+(pulse-saw)*(shape-.3333f)*3.f;
  return pulse+(tri-pulse)*(shape-.6667f)*3.f;
}
struct Params {
 float shape=.55f, fm=.30f, detune=.20f, sub=.50f;
 float cutoff=2400, resonance=.18f, vowel=.45f, movement=.55f, rate=2, drive=.50f;
 float attack=.005f, decay=.17f, sustain=.65f, release=.18f;
 float stereo=.35f, level=.50f, mix=.95f, glide=.0f;
 bool mono=true;
};
struct StateVariableFilter {
 float ic1=0,ic2=0;
 void reset(){ic1=ic2=0;}
 // TPT state variable filter, returns lowpass and bandpass.
 void tick(float x,float hz,float res,float sr,float &lp,float &bp){
  const float g=std::tan(pi*clamp(hz,25.f,sr*.43f)/sr);
  const float k=2.0f-1.88f*clamp(res,0.f,1.f);
  const float a=1.f/(1.f+g*(g+k));
  const float v1=a*(ic1+g*(x-ic2));
  const float v2=ic2+g*v1;
  ic1=2*v1-ic1;ic2=2*v2-ic2;
  lp=v2;bp=v1;
 }
};
struct Voice {
 int note=-1; bool holding=false; int stage=0; float env=0,velocity=1;
 float phase1=0,phase2=0,phaseFM=0,phaseSub=0, freq=55, target=55;
 float lfo=0, drift=0, pan=0; uint32_t rng=1234567;
 StateVariableFilter main,form1,form2;
 void reset(){*this=Voice{};}
 float randf(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (rng/4294967295.f)*2.f-1.f;}
 void start(int n,float vel,float offset,float hz, bool legato){
  if(!legato){phase1=phase2=phaseFM=phaseSub=0;main.reset();form1.reset();form2.reset();env=0;stage=1;freq=hz;}
  target=hz;note=n;holding=true;velocity=clamp(vel,.02f,1.f);pan=offset;
 }
 void stop(){holding=false;if(stage!=0)stage=4;}
 float envelope(const Params&p,float sr){
  switch(stage){
    case 1:env+=1.f/(std::max(.001f,p.attack)*sr);if(env>=1){env=1;stage=2;}break;
    case 2:env+=(p.sustain-env)*std::min(1.f,4.f/(std::max(.001f,p.decay)*sr));if(env<=p.sustain+.002f)stage=3;break;
    case 3:env=p.sustain;break;
    case 4:env*=std::exp(-6.9f/(std::max(.01f,p.release)*sr));if(env<.00015f){env=0;stage=0;note=-1;}break;
    default:break;
  }
  return env;
 }
};
struct Engine {
 static constexpr int maxVoices=12;
 std::array<Voice,maxVoices> voices{};
 Params p{};float sr=48000,beatBpm=140;uint64_t samples=0;int nextVoice=0;
 void prepare(float rate){sr=std::max(8000.f,rate); samples=0;for(auto &v:voices)v.reset();}
 void setTempo(float bpm){beatBpm=clamp(bpm,30.f,300.f);}
 void noteOn(int n,float vel){
  n=std::max(0,std::min(n,127));const float hz=440.f*std::pow(2.f,(n-69)/12.f);
  if(p.mono){auto &v=voices[0];v.start(n,vel,0,hz,v.stage!=0 && p.glide>.001f);return;}
  Voice* v=nullptr;for(auto &a:voices)if(a.stage==0){v=&a;break;}
  if(!v){v=&voices[static_cast<size_t>(nextVoice++%maxVoices)];}
  v->start(n,vel,(nextVoice%2==0?-.35f:.35f),hz,false);
 }
 void noteOff(int n){for(auto &v:voices)if(v.note==n && v.holding)v.stop();}
 void allNotesOff(){for(auto &v:voices)v.stop();}
 // One output sample; internally 4x oversampled nonlinear synthesis and filters.
 std::array<float,2> tick(){
  float l=0,r=0;const float osr=sr*4.f;
  for(auto &v:voices){if(v.stage==0)continue;
    const float envelope=v.envelope(p,sr);if(v.stage==0)continue;
    float sum=0;
    for(int k=0;k<4;++k){
      const float glideCoeff=p.glide<=.001f?1.f:1.f-std::exp(-1.f/(p.glide*osr));
      v.freq+=(v.target-v.freq)*glideCoeff;
      const float beatRate=(p.rate<=.01f?0.f:(beatBpm/60.f)*p.rate);
      v.lfo=fract(v.lfo+beatRate/osr);
      const float mod=.5f-.5f*std::cos(2*pi*v.lfo);
      const float waveMod=p.movement*(mod-.5f);
      const float freq=std::min(v.freq,osr*.20f);
      const float d1=freq/osr, d2=std::min(freq*(1.f+p.detune*.018f),osr*.25f)/osr;
      v.phaseFM=fract(v.phaseFM+d1*(1.5f+6.f*p.fm));
      const float fm=std::sin(2*pi*v.phaseFM)*p.fm*2.2f;
      v.phase1=fract(v.phase1+d1);
      v.phase2=fract(v.phase2+d2);
      const float sh=clamp(p.shape+waveMod,0,1);
      const float a=osc(fract(v.phase1+fm*.10f),d1,sh);
      const float b=osc(fract(v.phase2-fm*.065f),d2,clamp(sh+.24f,0,1));
      float mid=.60f*a+.40f*b;
      const float cutoff=clamp(p.cutoff*std::pow(2.f,waveMod*4.f),60.f,osr*.30f);
      float lp,bp;v.main.tick(mid,cutoff,p.resonance,osr,lp,bp);
      // Parallel animated formant bandpasses, with significant modulation depth.
      float f1,b1,f2,b2;
      const float vowel=clamp(p.vowel + p.movement*(mod-.5f)*.9f,0,1);
      v.form1.tick(mid,350.f+vowel*1350.f,.80f,osr,f1,b1);
      v.form2.tick(mid,850.f+(1-vowel)*2350.f,.83f,osr,f2,b2);
      const float formant=(b1*.9f+b2*.7f);
      mid=(lp*(1.f-.65f*p.vowel)+formant*(.3f+1.55f*p.vowel));
      // Soft, oversampled drive; keep separate sub out of this stage.
      const float drive=1.f+p.drive*19.f;
      const float wet=sat(mid*drive)/(std::max(.1f,sat(drive)));
      mid=(1.f-p.mix)*mid+p.mix*wet;
      v.phaseSub=fract(v.phaseSub+d1*.5f);
      const float sub=std::sin(2*pi*v.phaseSub)*p.sub*.65f;
      sum+=(mid*.72f+sub)*.25f;
    }
    const float amplitude=envelope*v.velocity*v.velocity*p.level*.55f;
    const float pan=clamp(v.pan*p.stereo,-.8f,.8f);
    l+=sum*amplitude*(1.f-pan*.5f);
    r+=sum*amplitude*(1.f+pan*.5f);
  }
  ++samples;
  // Gently limit summing overs to avoid digital overs, not a mastering limiter.
  return {sat(l*.90f),sat(r*.90f)};
 }
};
} // namespace rage
