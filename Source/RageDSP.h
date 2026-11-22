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
// Tempo divisions in quarter-note beats; 1 BAR means four beats in 4/4.
inline float lfoBeatsPerCycle(int division){
 constexpr float beats[]{.125f,.25f,.5f,1.f,2.f,4.f,8.f};
 return beats[std::max(0,std::min(6,division))];
}
inline float lfoValue(float phase,int shape){
 const float x=fract(phase);
 switch(shape){
  case 1:return 1.f-std::abs(2.f*x-1.f); // triangle
  case 2:return x;                       // rising saw
  case 3:return 1.f-x;                   // falling saw
  case 4:return x<.5f?1.f:0.f;         // square
  default:return .5f-.5f*std::cos(2.f*pi*x); // sine
 }
}
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
 int lfoDivision=3,lfoShape=0;
 float attack=.005f, decay=.17f, sustain=.65f, release=.18f;
 float stereo=.35f, level=.50f, mix=.95f, glide=.0f, headroomDb=3.f;
 float waveBlend=0.f,waveWarp=0.f;
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
 int scopeCycle=0; // Tracks oscillator periods for the eight-cycle waveform preview.
 float lfo=0, drift=0, pan=0, retriggerPitchFade=0; uint32_t rng=1234567;
 StateVariableFilter main,form1,form2;
 void reset(){*this=Voice{};}
 float randf(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return (rng/4294967295.f)*2.f-1.f;}
 void start(int n,float vel,float offset,float hz, bool legato){
  const bool active=stage!=0;
  if(!legato && !active){
    phase1=phase2=phaseFM=phaseSub=0;scopeCycle=0;
    main.reset();form1.reset();form2.reset();env=0;stage=1;freq=hz;
    retriggerPitchFade=0.f;
  }else if(!legato && active){
    // On repeated notes, preserve the oscillator and filter continuity.
    // Resetting an active mono voice produced an audible one-sample step.
    stage=1;retriggerPitchFade=1.f;
  }
  target=hz;note=n;holding=true;velocity=clamp(vel,.02f,1.f);pan=offset;
 }
 void stop(){holding=false;if(stage!=0)stage=4;}
 float envelope(const Params&p,float sr){
  switch(stage){
    case 1:env+=1.f/(std::max(.003f,p.attack)*sr);if(env>=1){env=1;stage=2;}break;
    case 2:env+=(p.sustain-env)*std::min(1.f,4.f/(std::max(.001f,p.decay)*sr));if(env<=p.sustain+.002f)stage=3;break;
    case 3:env=p.sustain;break;
    case 4:env*=std::exp(-6.9f/(std::max(.008f,p.release)*sr));if(env<.00015f){env=0;stage=0;note=-1;}break;
    default:break;
  }
  return env;
 }
};
struct Engine {
 static constexpr int maxVoices=12;
 static constexpr int waveSize=64;
 std::array<Voice,maxVoices> voices{};
 std::array<float,waveSize> wave{},waveTarget{};
 Engine(){for(int i=0;i<waveSize;++i)wave[i]=waveTarget[i]=std::sin(2.f*pi*(float)i/waveSize);}
 void setWavePoint(int index,float value){if(index>=0&&index<waveSize)waveTarget[(size_t)index]=clamp(value,-1.f,1.f);}
 float customWave(float phase,float warp) const {
   const float warped=fract(phase+clamp(warp,0.f,1.f)*.18f*std::sin(2.f*pi*phase));
   const float index=warped*(float)waveSize;
   const int i=(int)index;
   const float f=index-(float)i;
   return wave[(size_t)(i%waveSize)]*(1.f-f)+wave[(size_t)((i+1)%waveSize)]*f;
 }
 Params p{};float sr=48000,beatBpm=140;uint64_t samples=0;int nextVoice=0;
 void prepare(float rate){sr=std::max(8000.f,rate); samples=0;for(auto &v:voices)v.reset();}
 void setTempo(float bpm){beatBpm=clamp(bpm,30.f,300.f);}
 void noteOn(int n,float vel){
  n=std::max(0,std::min(n,127));const float hz=440.f*std::pow(2.f,(n-69)/12.f);
  if(p.mono){
   auto &v=voices[0];
   // Legato applies only while the previous MIDI note is actually held.
   // A voice in its release tail must start a fresh envelope on the next note.
   // Otherwise pitch changes can sound like a delayed double hit or "flam".
   const bool trueLegato=v.holding && v.stage!=0 && p.glide>.001f;
   v.start(n,vel,0,hz,trueLegato);
   return;
  }
  Voice* v=nullptr;for(auto &a:voices)if(a.stage==0){v=&a;break;}
  if(!v){v=&voices[static_cast<size_t>(nextVoice++%maxVoices)];}
  v->start(n,vel,(nextVoice%2==0?-.35f:.35f),hz,false);
 }
 void noteOff(int n){for(auto &v:voices)if(v.note==n && v.holding)v.stop();}
 void allNotesOff(){for(auto &v:voices)v.stop();}
 // One output sample; internally 4x oversampled nonlinear synthesis and filters.
 std::array<float,2> tick(){
  float l=0,r=0;const float osr=sr*4.f;
  // Smooth live edits to avoid audio discontinuities as the wave is drawn.
  const float waveSlew=1.f-std::exp(-1.f/(.012f*sr));
  for(int i=0;i<waveSize;++i)wave[(size_t)i]+=(waveTarget[(size_t)i]-wave[(size_t)i])*waveSlew;
  for(auto &v:voices){if(v.stage==0)continue;
    const float envelope=v.envelope(p,sr);if(v.stage==0)continue;
    float sum=0;
    for(int k=0;k<4;++k){
      const float glideTime=v.retriggerPitchFade>0.f?std::max(.003f,p.glide):p.glide;
      const float glideCoeff=glideTime<=.001f?1.f:1.f-std::exp(-1.f/(glideTime*osr));
      v.freq+=(v.target-v.freq)*glideCoeff;
      v.retriggerPitchFade=std::max(0.f,v.retriggerPitchFade-1.f/(.003f*osr));
      const float beatRate=(beatBpm/60.f)/lfoBeatsPerCycle(p.lfoDivision);
      v.lfo=fract(v.lfo+beatRate/osr);
      const float mod=lfoValue(v.lfo,p.lfoShape);
      const float waveMod=p.movement*(mod-.5f);
      const float freq=std::min(v.freq,osr*.20f);
      const float d1=freq/osr, d2=std::min(freq*(1.f+p.detune*.018f),osr*.25f)/osr;
      v.phaseFM=fract(v.phaseFM+d1*(1.5f+6.f*p.fm));
      const float fm=std::sin(2*pi*v.phaseFM)*p.fm*2.2f;
      const float nextPhase1=v.phase1+d1;
      if(nextPhase1>=1.f)v.scopeCycle=(v.scopeCycle+1)%8;
      v.phase1=fract(nextPhase1);
      v.phase2=fract(v.phase2+d2);
      const float sh=clamp(p.shape+waveMod,0,1);
      const float mainPhase=fract(v.phase1+fm*.10f);
      const float a=(1.f-p.waveBlend)*osc(mainPhase,d1,sh)+p.waveBlend*customWave(mainPhase,p.waveWarp);
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
  // Post-saturation master attenuation and safety ceiling (-1 dBFS peak).
  // Headroom reduces summing peaks without altering the oscillator settings.
  const float gain=std::pow(10.f,-clamp(p.headroomDb,0.f,18.f)/20.f);
  constexpr float ceiling=.891250938f;
  // Smooth soft ceiling rather than hard clipping, which would add new clicks.
  return {ceiling*sat(sat(l*.90f)*gain/ceiling),
          ceiling*sat(sat(r*.90f)*gain/ceiling)};
 }
};
} // namespace rage
