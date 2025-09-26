#pragma once
#include <algorithm>
#include <vector>

template <typename... Args> class EventSink {
public:
  virtual ~EventSink() = default;
  virtual void onEvent(Args... args) = 0;
};

template <typename... Args> class EventEmitter {
  std::vector<EventSink<Args...> *> sinks;

public:
  void emit(Args... args) {
    for (auto *sink : sinks) {
      sink->onEvent(args...);
    }
  }

  EventEmitter &operator+=(EventSink<Args...> &sink) {
    sinks.push_back(&sink);
    return *this;
  }

  EventEmitter &operator-=(EventSink<Args...> &sink) {
    sinks.erase(std::remove(sinks.begin(), sinks.end(), &sink), sinks.end());
    return *this;
  }
};
