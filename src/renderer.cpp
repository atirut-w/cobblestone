#include "renderer.hpp"
#include "engine.hpp"
#include <SDL_opengl.h>
#include <format>
#include <stdexcept>

void Renderer::init(const char *title, int width, int height) {
  if (initialized) {
    logger->warn("Renderer is already initialized");
    return;
  }

  // Do not *ever* use ES 1.1 on PC, or no triangles will render!
  // Tested on:
  // - Mesa Intel(R) Iris(R) Xe Graphics (TGL GT2)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  logger->info("Creating window...");
  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!window) {
    throw std::runtime_error(
        std::format("Failed to create window: {}", SDL_GetError()));
  }
  resolution = Vector2<int>(width, height);

  logger->info("Creating OpenGL context...");
  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    SDL_DestroyWindow(window);
    throw std::runtime_error(
        std::format("Failed to create OpenGL context: {}", SDL_GetError()));
  }

  logger->info("OpenGL Vendor: {}", (const char *)glGetString(GL_VENDOR));
  logger->info("OpenGL Renderer: {}", (const char *)glGetString(GL_RENDERER));
  logger->info("OpenGL Version: {}", (const char *)glGetString(GL_VERSION));

  Engine::get().eventEmitter += *this;
  initialized = true;
}

void Renderer::shutdown() {
  if (!initialized) {
    logger->warn("Renderer is not initialized");
    return;
  }

  if (gl_context) {
    SDL_GL_DeleteContext(gl_context);
    gl_context = nullptr;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  initialized = false;
}

void Renderer::swapBuffers() {
  if (!initialized) {
    logger->warn("Renderer is not initialized");
    return;
  }

  SDL_GL_SwapWindow(window);
}

void Renderer::onEvent(SDL_Event &event) {
  if (event.type == SDL_WINDOWEVENT) {
    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
      int width = event.window.data1;
      int height = event.window.data2;
      glViewport(0, 0, width, height);
      resolution = Vector2<int>(width, height);
      logger->info("Window resized to {}x{}", width, height);
    }
  }
}

void Renderer::beginUI() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, resolution.x, resolution.y, 0, -1, 1);
  glPushMatrix();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPushMatrix();

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::end() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}
