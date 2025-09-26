#include "renderer.hpp"
#include <SDL_opengl.h>
#include <format>
#include <stdexcept>

void Renderer::init(const char *title, int width, int height) {
  if (initialized) {
    logger->warn("Renderer is already initialized");
    return;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

  logger->info("Creating window...");
  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       width, height, SDL_WINDOW_OPENGL);
  if (!window) {
    throw std::runtime_error(
        std::format("Failed to create window: {}", SDL_GetError()));
  }

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
