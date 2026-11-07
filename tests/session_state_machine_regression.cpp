#include "training/SessionStateMachine.hpp"
#include <cassert>
using biofeedback::training::SessionState;
using biofeedback::training::SessionStateMachine;
int main(){
 SessionStateMachine s;
 assert(s.state()==SessionState::Idle && s.canStart());
 assert(!s.pause() && !s.resume() && !s.beginRest());
 assert(s.start() && s.state()==SessionState::Active);
 assert(!s.start());
 assert(s.pause() && s.state()==SessionState::Paused);
 assert(!s.pause() && s.resume() && s.state()==SessionState::Active);
 assert(s.beginRest() && s.state()==SessionState::Resting && s.resting());
 assert(s.pause() && s.paused() && s.resting());
 assert(s.resume() && s.state()==SessionState::Resting);
 assert(s.endRest() && s.state()==SessionState::Active);
 assert(s.interrupt() && s.state()==SessionState::Interrupted && s.canStart());
 assert(!s.interrupt() && !s.complete());
 assert(s.start() && s.state()==SessionState::Active);
 assert(s.complete() && s.state()==SessionState::Completed && s.canStart());
 assert(s.start() && s.state()==SessionState::Active);
 assert(s.interrupt() && s.state()==SessionState::Interrupted);
 return 0;
}
