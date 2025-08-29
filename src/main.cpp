#include <GLES/gl.h>
#include <SDL.h>
#include <stdexcept>

#ifdef __PSP__
#include <exception>
#include <pspdebug.h>
#include <pspdisplay.h>

[[noreturn]] void terminate() noexcept {
  pspDebugScreenInit();
  pspDebugScreenSetXY(0, 0);

  if (auto ex = std::current_exception()) {
    try {
      std::rethrow_exception(ex);
    } catch (const std::exception &e) {
      pspDebugScreenPrintf("%s\n", e.what());
    } catch (...) {
      pspDebugScreenPrintf("Unknown exception\n");
    }
  } else {
    pspDebugScreenPrintf("Terminate called without exception\n");
  }

  while (1)
    sceDisplayWaitVblankStart();
}
#endif

int main(int argc, char *argv[]) {
#ifdef __PSP__
  std::set_terminate(terminate);
#endif

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
