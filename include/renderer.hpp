#pragma once
#include "events.hpp"
#include "singleton.hpp"
#include <SDL.h>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>

class Renderer : public Singleton<Renderer>, public EventSink<SDL_Event &> {
  std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt("Renderer");
  SDL_Window *window = nullptr;
  SDL_GLContext gl_context = nullptr;
  bool initialized = false;

public:

  void init(const char *title, int width, int height);
  void shutdown();
  void swapBuffers();
  void onEvent(SDL_Event &event) override;
};
