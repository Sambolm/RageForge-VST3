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
 std::cout<<"PASS: finite stereo synthesis, output ceiling, note-off release, strongly differentiated timbres, polyphony, sub bass. diff="<<diff<<"\n";
}
