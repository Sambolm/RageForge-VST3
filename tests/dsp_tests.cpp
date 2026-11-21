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
 std::cout<<"PASS: finite stereo synthesis, output ceiling, note-off release, strongly differentiated timbres, polyphony, sub bass. diff="<<diff<<"\n";
}
