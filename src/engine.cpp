#include "engine.hpp"
#include "renderer.hpp"
#include "texture.hpp"
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
      eventEmitter.emit(event);
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    // TODO: Main loop logic
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer.beginUI();
    glBegin(GL_QUADS);
    glShadeModel(GL_SMOOTH);
    glColor3f(1.0f, 0.0f, 0.0f); // Red (top-left)
    glVertex2f(0.0f, 0.0f);

    glColor3f(1.0f, 1.0f, 0.0f); // Yellow (top-right)
    glVertex2f(80.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f); // Green (bottom-right)
    glVertex2f(80.0f, 80.0f);

    glColor3f(0.0f, 0.0f, 1.0f); // Blue (bottom-left)
    glVertex2f(0.0f, 80.0f);
    glEnd();
    renderer.end();

    renderer.swapBuffers();
  }

  renderer.shutdown();
  SDL_Quit();

  return 0;
}
