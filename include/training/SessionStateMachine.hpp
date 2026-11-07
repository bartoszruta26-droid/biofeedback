#pragma once

namespace biofeedback::training {

enum class SessionState { Idle, Active, Paused, Resting, Completed, Interrupted };
enum class RestTarget { NextSeries, NextExercise };

class SessionStateMachine {
public:
    SessionState state() const noexcept { return state_; }
    SessionState resumeState() const noexcept { return resumeState_; }
    RestTarget restTarget() const noexcept { return restTarget_; }

    bool restore(SessionState state, SessionState resumeState = SessionState::Active,
                 RestTarget restTarget = RestTarget::NextSeries) noexcept {
        const bool validState = state == SessionState::Idle || state == SessionState::Active ||
            state == SessionState::Paused || state == SessionState::Resting ||
            state == SessionState::Completed || state == SessionState::Interrupted;
        const bool validResume = resumeState == SessionState::Active ||
            resumeState == SessionState::Resting;
        const bool validRestTarget = restTarget == RestTarget::NextSeries ||
            restTarget == RestTarget::NextExercise;
        if (!validState || !validResume || !validRestTarget) return false;
        if (state == SessionState::Resting && resumeState != SessionState::Active) return false;
        state_ = state;
        resumeState_ = resumeState;
        restTarget_ = restTarget;
        return true;
    }

    bool canStart() const noexcept {
        return state_ == SessionState::Idle || state_ == SessionState::Completed ||
            state_ == SessionState::Interrupted;
    }

    bool start() noexcept {
        if (!canStart()) return false;
        state_ = SessionState::Active;
        resumeState_ = SessionState::Active;
        restTarget_ = RestTarget::NextSeries;
        return true;
    }

    bool pause() noexcept {
        if (state_ != SessionState::Active && state_ != SessionState::Resting) return false;
        resumeState_ = state_;
        state_ = SessionState::Paused;
        return true;
    }

    bool resume() noexcept {
        if (state_ != SessionState::Paused) return false;
        state_ = resumeState_;
        return true;
    }

    bool beginRest(RestTarget target = RestTarget::NextSeries) noexcept {
        if (state_ != SessionState::Active) return false;
        restTarget_ = target;
        state_ = SessionState::Resting;
        return true;
    }

    bool endRest() noexcept {
        if (state_ != SessionState::Resting) return false;
        state_ = SessionState::Active;
        restTarget_ = RestTarget::NextSeries;
        return true;
    }

    bool complete() noexcept {
        if (state_ != SessionState::Active && state_ != SessionState::Resting &&
            state_ != SessionState::Paused) return false;
        state_ = SessionState::Completed;
        resumeState_ = SessionState::Active;
        restTarget_ = RestTarget::NextSeries;
        return true;
    }

    bool interrupt() noexcept {
        if (state_ != SessionState::Active && state_ != SessionState::Resting &&
            state_ != SessionState::Paused) return false;
        state_ = SessionState::Interrupted;
        resumeState_ = SessionState::Active;
        restTarget_ = RestTarget::NextSeries;
        return true;
    }

    bool running() const noexcept {
        return state_ == SessionState::Active || state_ == SessionState::Paused ||
            state_ == SessionState::Resting;
    }

    bool paused() const noexcept { return state_ == SessionState::Paused; }
    bool resting() const noexcept {
        return state_ == SessionState::Resting ||
            (state_ == SessionState::Paused && resumeState_ == SessionState::Resting);
    }

private:
    SessionState state_{SessionState::Idle};
    SessionState resumeState_{SessionState::Active};
    RestTarget restTarget_{RestTarget::NextSeries};
};

} // namespace biofeedback::training
