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

 // A process restart restores an in-progress workout as paused. The caller
 // explicitly resumes into the exact prior phase, including an active rest.
 SessionStateMachine recovered;
 assert(recovered.restore(SessionState::Paused, SessionState::Active));
 assert(recovered.running() && recovered.paused() && !recovered.resting());
 assert(recovered.resume() && recovered.state()==SessionState::Active);
 assert(recovered.beginRest());
 assert(recovered.restore(SessionState::Paused, SessionState::Resting));
 assert(recovered.running() && recovered.paused() && recovered.resting());
 assert(recovered.resume() && recovered.state()==SessionState::Resting);
 // Recovery always starts paused so a process restart cannot implicitly
 // restart the timer or deliver live sensor samples before user confirmation.
 assert(!recovered.restore(SessionState::Paused, SessionState::Completed));
 assert(!recovered.restore(static_cast<SessionState>(99), SessionState::Active));
 return 0;
}
