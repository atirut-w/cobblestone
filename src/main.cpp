#include <GLES/gl.h>
#include <SDL.h>
#include <stdexcept>

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
    throw std::runtime_error("Failed to initialize SDL");
  }

  SDL_Window *window =
      SDL_CreateWindow("Cobblestone", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 480, 272, SDL_WINDOW_OPENGL);
  if (!window) {
    throw std::runtime_error("Failed to create SDL window");
  }

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    throw std::runtime_error("Failed to create OpenGL context");
  }

  bool running = true;
  SDL_Event event;
  while (running) {
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    glClearColor(1.0, 1.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
