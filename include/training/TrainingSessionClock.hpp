#pragma once
#include <cstdint>
namespace biofeedback::training {
class TrainingSessionClock {
public:
 void reset() noexcept { last_=0; active_=0; initialized_=false; }
 void update(std::int64_t elapsed, bool isActive) noexcept {
  if(elapsed<0) elapsed=0;
  if(!initialized_){last_=elapsed;initialized_=true;return;}
  if(elapsed<=last_) return;
  if(isActive) active_+=elapsed-last_;
  last_=elapsed;
 }
 std::int64_t sessionMilliseconds() const noexcept { return initialized_?last_:0; }
 std::int64_t activeMilliseconds() const noexcept { return active_; }
 std::int64_t sessionSeconds() const noexcept { return sessionMilliseconds()/1000; }
 std::int64_t activeSeconds() const noexcept { return active_/1000; }
private:
 std::int64_t last_{0},active_{0}; bool initialized_{false};
};
}
