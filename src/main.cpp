#include "font.hpp"
#include <GLES/gl.h>
#include <SDL.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <utf8.h>

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


void render_text(const Font& font, const std::string& text, float x, float y, float scale = 1.0f) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float current_x = x;
    auto text_it = text.begin();

    while (text_it != text.end()) {
        uint32_t codepoint = 0;
        try {
            codepoint = utf8::next(text_it, text.end());
        } catch (const utf8::exception&) {
            break; // Invalid UTF-8
        }

        uint16_t code = (codepoint <= 0xFFFF) ? static_cast<uint16_t>(codepoint) : 0;
        if (code == 0) continue;

        // Find the provider that has this character
        const BitmapFontProvider* bitmap_provider = nullptr;
        const BitmapFontProvider::Glyph* glyph = nullptr;

        // Function to search providers recursively
        std::function<void(const std::vector<std::unique_ptr<FontProvider>>&)> search_providers =
            [&](const std::vector<std::unique_ptr<FontProvider>>& providers) {
                for (const auto& provider : providers) {
                    if (auto bmp = dynamic_cast<const BitmapFontProvider*>(provider.get())) {
                        // Search through glyph cache
                        auto it = bmp->glyphs.find(code);
                        if (it != bmp->glyphs.end()) {
                            bitmap_provider = bmp;
                            glyph = &it->second;
                            return;
                        }
                    } else if (auto ref = dynamic_cast<const ReferenceFontProvider*>(provider.get())) {
                        search_providers(ref->ref->providers);
                    }
                    if (bitmap_provider) return;
                }
            };

        search_providers(font.providers);

        if (bitmap_provider && glyph) {
            // Use cached glyph coordinates
            float u1 = glyph->u;
            float v1 = glyph->v;
            float u2 = u1 + (glyph->width / static_cast<float>(bitmap_provider->texture_width));
            float v2 = v1 + (bitmap_provider->height / static_cast<float>(bitmap_provider->texture_height));

            // Bind texture
            glBindTexture(GL_TEXTURE_2D, bitmap_provider->texture);

            // Draw quad
            glBegin(GL_QUADS);
            glTexCoord2f(u1, v1); glVertex2f(current_x, y);
            glTexCoord2f(u2, v1); glVertex2f(current_x + glyph->width * scale, y);
            glTexCoord2f(u2, v2); glVertex2f(current_x + glyph->width * scale, y + bitmap_provider->height * scale);
            glTexCoord2f(u1, v2); glVertex2f(current_x, y + bitmap_provider->height * scale);
            glEnd();

            // Advance position
            current_x += glyph->width * scale;
        } else {
            // Check space provider for advance (recursively search through all providers)
            int advance = 0;
            bool found_advance = false;
            
            std::function<void(const std::vector<std::unique_ptr<FontProvider>>&)> search_space_providers =
                [&](const std::vector<std::unique_ptr<FontProvider>>& providers) {
                    for (const auto& provider : providers) {
                        if (auto space = dynamic_cast<const SpaceFontProvider*>(provider.get())) {
                            auto it = space->advances.find(code);
                            if (it != space->advances.end()) {
                                advance = it->second;
                                found_advance = true;
                                return;
                            }
                        } else if (auto ref = dynamic_cast<const ReferenceFontProvider*>(provider.get())) {
                            search_space_providers(ref->ref->providers);
                        }
                        if (found_advance) return;
                    }
                };
            
            search_space_providers(font.providers);

            if (found_advance) {
                current_x += advance * scale;
            }
        }
    }

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

int main(int argc, char *argv[]) {
#ifdef __PSP__
  std::set_terminate(terminate);
#endif

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
    throw std::runtime_error("Failed to initialize SDL");
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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

  Font font(ResourceLocation("minecraft:default"));
  std::cout << "Font loaded with " << font.providers.size() << " providers" << std::endl;

  bool running = true;
  SDL_Event event;
  while (running) {
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    glClearColor(0.5, 0.5, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up 2D projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, 480, 272, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render some text
    render_text(font, "Hello World!", 50, 50, 2.0f);
    render_text(font, "Font Rendering Test", 50, 100, 1.5f);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

