#ifndef SIMPLE_TIMER_H_
#define SIMPLE_TIMER_H_

#if __cplusplus < 201103L && (!defined(_MSC_VER) || _MSC_VER < 1700)
#error C++11 support is required
#endif

#include <chrono>
#include <ostream>

namespace timer {

struct clock_not_started : std::exception {
  // clock_not_started() : std::exception("Clock not started.") {}
  clock_not_started() = default;
};

template<class Clock>
class CustomTimer {
 public:
  CustomTimer(Clock& clock) : clock_(clock) {}
  
  void Start() {
    if (started_)
      return;
    started_ = true;
    start_ = clock_;
  }

  template<class T = Clock>
  T Stop() {
    if (!started_)
      throw clock_not_started();
    started_ = false;
    return clock_ - start_;
  }

  void Reset() {
    started_ = false;
    // start_ = clock_;
  }

  template<class T = Clock>
  T Elapsed() const {
    return static_cast<T>(clock_ - start_);
  }

  template <typename T, typename Traits>
  friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const CustomTimer& timer) {
    return out << timer.Elapsed();
  }

 private:
  Clock start_, &clock_;
  bool started_ = false;
};

template<class Clock>
CustomTimer<Clock> make_clock(Clock& clock) {
  return CustomTimer<Clock>(clock);
}

class SimpleTimer {
 public:
  using clock = std::chrono::steady_clock;
  using milliseconds = std::chrono::milliseconds;

  SimpleTimer() = default;
  
  void Reset() {
    start_ = clock::now();
  }

  template<class T = milliseconds>
  T Elapsed() const {
    return std::chrono::duration_cast<T>(clock::now() - start_);
  }

  template <typename T, typename Traits>
  friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const SimpleTimer& timer) {
    return out << timer.Elapsed().count() << " ms";
  }

 private:
  clock::time_point start_;
};

}  // timer

#endif  // SIMPLE_TIMER_H_
