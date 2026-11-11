#include "training/SessionStateMachine.hpp"
#include <cassert>
using biofeedback::training::SessionState;
using biofeedback::training::SessionStateMachine;
using biofeedback::training::RestTarget;
int main() {
    SessionStateMachine machine;
    assert(machine.state() == SessionState::Idle && machine.canStart());
    assert(!machine.pause() && !machine.resume() && !machine.beginRest());
    assert(machine.start() && machine.state() == SessionState::Active);
    assert(!machine.start());
    assert(machine.pause() && machine.state() == SessionState::Paused);
    assert(!machine.pause() && machine.resume() && machine.state() == SessionState::Active);

    // Rest between sets preserves its target while paused and after recovery.
    assert(machine.beginRest(RestTarget::NextSeries));
    assert(machine.resting() && machine.restTarget() == RestTarget::NextSeries);
    assert(machine.pause() && machine.paused() && machine.resting());
    assert(machine.resume() && machine.state() == SessionState::Resting);
    assert(machine.endRest() && machine.state() == SessionState::Active);
    assert(machine.restTarget() == RestTarget::NextSeries);

    // Rest between exercises has a different continuation target.
    assert(machine.beginRest(RestTarget::NextExercise));
    assert(machine.restTarget() == RestTarget::NextExercise);
    SessionStateMachine recovered;
    assert(recovered.restore(SessionState::Paused, SessionState::Resting, RestTarget::NextExercise));
    assert(recovered.running() && recovered.paused() && recovered.resting());
    assert(recovered.restTarget() == RestTarget::NextExercise);
    assert(recovered.resume() && recovered.state() == SessionState::Resting);
    assert(recovered.endRest() && recovered.state() == SessionState::Active);
    assert(recovered.restTarget() == RestTarget::NextSeries);

    assert(machine.interrupt() && machine.state() == SessionState::Interrupted && machine.canStart());
    assert(!machine.interrupt() && !machine.complete());
    assert(machine.start() && machine.complete());
    assert(machine.state() == SessionState::Completed && machine.canStart());

    // Invalid enum values and incompatible restoration are rejected.
    assert(!recovered.restore(SessionState::Paused, SessionState::Completed));
    assert(!recovered.restore(static_cast<SessionState>(99), SessionState::Active));
    assert(!recovered.restore(SessionState::Paused, SessionState::Active,
                              static_cast<RestTarget>(99)));
    return 0;
}
