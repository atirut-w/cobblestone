#include "engine.hpp"
#include "renderer.hpp"
#include <SDL.h>
#include <SDL_opengl.h>

int Engine::start(int argc, char *argv[]) {
  logger->info("Initializing SDL...");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) !=
      0) {
    logger->critical("Failed to initialize SDL: {}", SDL_GetError());
    return 1;
  }

  auto &renderer = Renderer::get();
  renderer.init("Cobblestone Engine", 480, 272);

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    // TODO: Main loop logic
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer.swapBuffers();
  }

  renderer.shutdown();
  SDL_Quit();

  return 0;
}
