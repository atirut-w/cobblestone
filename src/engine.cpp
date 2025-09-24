#include "engine.hpp"
#include <SDL.h>
#include <SDL_opengl.h>

int Engine::start(int argc, char *argv[]) {
  logger->info("Initializing SDL...");
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) !=
      0) {
    logger->critical("Failed to initialize SDL: {}", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  logger->info("Creating window...");
  SDL_Window *window =
      SDL_CreateWindow("Cobblestone Engine", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 480, 272, SDL_WINDOW_OPENGL);
  if (!window) {
    logger->critical("Failed to create window: {}", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  logger->info("Creating OpenGL context...");
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    logger->critical("Failed to create OpenGL context: {}", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  logger->info("OpenGL Vendor: {}", (const char *)glGetString(GL_VENDOR));
  logger->info("OpenGL Renderer: {}", (const char *)glGetString(GL_RENDERER));
  logger->info("OpenGL Version: {}", (const char *)glGetString(GL_VERSION));

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

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
