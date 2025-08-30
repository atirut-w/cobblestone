#include "font.hpp"
#include <GLES/gl.h>
#include <SDL.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <functional>

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

// Helper function to decode UTF-8 to codepoint
uint32_t decode_utf8(const std::string& s, size_t& i) {
    if (i >= s.size()) return 0;
    uint32_t code = 0;
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (c < 0x80) {
        code = c;
        i += 1;
    } else if (c < 0xE0) {
        if (i + 1 >= s.size()) return 0;
        code = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i+1]) & 0x3F);
        i += 2;
    } else if (c < 0xF0) {
        if (i + 2 >= s.size()) return 0;
        code = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+2]) & 0x3F);
        i += 3;
    } else if (c < 0xF8) {
        if (i + 3 >= s.size()) return 0;
        code = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[i+2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+3]) & 0x3F);
        i += 4;
    } else {
        i += 1; // invalid
        return 0;
    }
    return code;
}

void render_text(const Font& font, const std::string& text, float x, float y, float scale = 1.0f) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float current_x = x;
    size_t text_i = 0;

    while (text_i < text.size()) {
        uint32_t codepoint = decode_utf8(text, text_i);
        if (codepoint == 0) break;

        uint16_t code = (codepoint <= 0xFFFF) ? static_cast<uint16_t>(codepoint) : 0;
        if (code == 0) continue;

        // Find the provider that has this character
        const BitmapFontProvider* bitmap_provider = nullptr;
        int char_index = -1;
        int row = -1, col = -1;

        // Function to search providers recursively
        std::function<void(const std::vector<std::unique_ptr<FontProvider>>&)> search_providers =
            [&](const std::vector<std::unique_ptr<FontProvider>>& providers) {
                for (const auto& provider : providers) {
                    if (auto bmp = dynamic_cast<const BitmapFontProvider*>(provider.get())) {
                        // Search through chars
                        for (int r = 0; r < bmp->chars.size(); ++r) {
                            const auto& row_chars = bmp->chars[r];
                            for (int c = 0; c < row_chars.size(); ++c) {
                                if (row_chars[c] == code) {
                                    bitmap_provider = bmp;
                                    row = r;
                                    col = c;
                                    return;
                                }
                            }
                        }
                    } else if (auto ref = dynamic_cast<const ReferenceFontProvider*>(provider.get())) {
                        search_providers(ref->ref->providers);
                    }
                    if (bitmap_provider) return;
                }
            };

        search_providers(font.providers);

        if (bitmap_provider) {
            // Calculate texture coordinates
            int tex_width = bitmap_provider->texture_width;
            int tex_height = bitmap_provider->texture_height;
            int chars_per_row = bitmap_provider->chars.empty() ? 16 : bitmap_provider->chars[0].size();
            int char_width = tex_width / chars_per_row;
            int char_height = bitmap_provider->height;

            float u1 = (col * char_width) / static_cast<float>(tex_width);
            float v1 = (row * char_height) / static_cast<float>(tex_height);
            float u2 = ((col + 1) * char_width) / static_cast<float>(tex_width);
            float v2 = ((row + 1) * char_height) / static_cast<float>(tex_height);

            // Bind texture
            glBindTexture(GL_TEXTURE_2D, bitmap_provider->texture);

            // Draw quad
            glBegin(GL_QUADS);
            glTexCoord2f(u1, v1); glVertex2f(current_x, y);
            glTexCoord2f(u2, v1); glVertex2f(current_x + char_width * scale, y);
            glTexCoord2f(u2, v2); glVertex2f(current_x + char_width * scale, y + char_height * scale);
            glTexCoord2f(u1, v2); glVertex2f(current_x, y + char_height * scale);
            glEnd();

            // Advance position
            current_x += char_width * scale;
        } else {
            // Check space provider for advance
            for (const auto& provider : font.providers) {
                if (auto space = dynamic_cast<const SpaceFontProvider*>(provider.get())) {
                    auto it = space->advances.find(code);
                    if (it != space->advances.end()) {
                        current_x += it->second * scale;
                        break;
                    }
                }
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

