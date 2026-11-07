#pragma once
namespace biofeedback::training {
enum class SessionState { Idle, Active, Paused, Resting, Completed, Interrupted };
class SessionStateMachine {
public:
 SessionState state() const noexcept { return state_; }
 SessionState resumeState() const noexcept { return resumeState_; }
 bool canStart() const noexcept { return state_==SessionState::Idle || state_==SessionState::Completed || state_==SessionState::Interrupted; }
 bool start() noexcept { if(!canStart()) return false; state_=SessionState::Active; resumeState_=SessionState::Active; return true; }
 bool pause() noexcept {
  if(state_!=SessionState::Active && state_!=SessionState::Resting) return false;
  resumeState_=state_; state_=SessionState::Paused; return true;
 }
 bool resume() noexcept { if(state_!=SessionState::Paused) return false; state_=resumeState_; return true; }
 bool beginRest() noexcept { if(state_!=SessionState::Active) return false; state_=SessionState::Resting; return true; }
 bool endRest() noexcept { if(state_!=SessionState::Resting) return false; state_=SessionState::Active; return true; }
 bool complete() noexcept {
  if(state_!=SessionState::Active && state_!=SessionState::Resting && state_!=SessionState::Paused) return false;
  state_=SessionState::Completed; resumeState_=SessionState::Active; return true;
 }
 bool interrupt() noexcept {
  if(state_!=SessionState::Active && state_!=SessionState::Resting && state_!=SessionState::Paused) return false;
  state_=SessionState::Interrupted; resumeState_=SessionState::Active; return true;
 }
 bool running() const noexcept { return state_==SessionState::Active || state_==SessionState::Paused || state_==SessionState::Resting; }
 bool paused() const noexcept { return state_==SessionState::Paused; }
 bool resting() const noexcept { return state_==SessionState::Resting || (state_==SessionState::Paused && resumeState_==SessionState::Resting); }
private:
 SessionState state_{SessionState::Idle};
 SessionState resumeState_{SessionState::Active};
};
}
