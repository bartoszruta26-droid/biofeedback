#include "training/TrainingSessionClock.hpp"
#include <cassert>
int main(){biofeedback::training::TrainingSessionClock c;c.reset();c.update(0,true);c.update(250,true);c.update(1250,false);c.update(3250,false);c.update(4250,true);assert(c.sessionMilliseconds()==4250);assert(c.activeMilliseconds()==1250);c.update(4250,true);c.update(4000,true);assert(c.activeMilliseconds()==1250);c.update(10250,true);assert(c.sessionSeconds()==10);assert(c.activeSeconds()==7);c.reset();assert(c.sessionMilliseconds()==0&&c.activeMilliseconds()==0);return 0;}
