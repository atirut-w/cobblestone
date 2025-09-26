#pragma once
#include "events.hpp"
#include "singleton.hpp"
#include <SDL.h>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Engine : public Singleton<Engine> {
  bool running = true;

public:
  std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Engine");
  EventEmitter<SDL_Event &> eventEmitter;

  int start(int argc, char *argv[]);
};
