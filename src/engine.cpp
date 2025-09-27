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
  SDL_Surface *tex = IMG_Load("assets/minecraft/textures/font/ascii.png");
  if (!tex) {
    logger->critical("Failed to load texture: {}", SDL_GetError());
    return 1;
  }
  Texture texture(*tex);
  SDL_FreeSurface(tex);

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
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    texture.bind();
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f); // Red (top-left)
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);

    glColor3f(1.0f, 1.0f, 0.0f); // Yellow (top-right)
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(256.0f, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f); // Green (bottom-right)
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(256.0f, 256.0f);

    glColor3f(0.0f, 0.0f, 1.0f); // Blue (bottom-left)
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, 256.0f);
    glEnd();
    renderer.end();

    renderer.swapBuffers();
  }

  renderer.shutdown();
  SDL_Quit();

  return 0;
}
