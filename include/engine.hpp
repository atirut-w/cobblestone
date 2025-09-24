#pragma once
#include "singleton.hpp"
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Engine : public Singleton<Engine> {
  bool running = true;

public:
  std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Engine");

  int start(int argc, char *argv[]);
};
