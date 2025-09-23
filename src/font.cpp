#include "font.hpp"
#include <GLES/gl.h>
#include <SDL.h>
#include <SDL_image.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <utf8.h>

using json = nlohmann::json;

Font::Font(const ResourceLocation &loc) {
  std::ifstream file(loc.to_path("font") + ".json");
  if (!file) {
    throw std::runtime_error("Failed to open font file " + std::string(loc));
  }
  json data = json::parse(file, nullptr, true, false, true);

  std::vector<std::string> referenced_fonts;
  for (const auto &provider : data["providers"]) {
    auto type = provider["type"].get<std::string>();
    if (type == "reference") {
      if (std::find(referenced_fonts.begin(), referenced_fonts.end(),
                    provider["id"].get<std::string>()) ==
          referenced_fonts.end()) {
        referenced_fonts.push_back(provider["id"].get<std::string>());
        Font refFont(ResourceLocation(provider["id"].get<std::string>()));
        for (auto &p : refFont.providers) {
          providers.push_back(std::move(p));
        }
      }
    } else if (type == "bitmap") {
      auto bitmapProvider = std::make_unique<BitmapFontProvider>();
      bitmapProvider->ascent = provider["ascent"].get<int>();
      if (provider.contains("height")) {
        bitmapProvider->height = provider["height"].get<int>();
      } else {
        bitmapProvider->height = 8;
      }
      std::string file_path =
          ResourceLocation(provider["file"].get<std::string>())
              .to_path("textures");
      SDL_Surface *surface = IMG_Load(file_path.c_str());
      if (!surface) {
        throw std::runtime_error("Failed to load texture " + file_path + ": " +
                                 IMG_GetError());
      }

      bitmapProvider->texture_width = surface->w;
      bitmapProvider->texture_height = surface->h;
      std::cout << "Loading texture: " << file_path << " ("
                << bitmapProvider->texture_width << "x"
                << bitmapProvider->texture_height << ")" << std::endl;
      std::cout << "Surface format: "
                << SDL_GetPixelFormatName(surface->format->format) << std::endl;

      // Convert to RGBA for OpenGL
      SDL_Surface *converted =
          SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
      if (!converted) {
        throw std::runtime_error("Failed to convert surface format");
      }
      SDL_FreeSurface(surface);
      surface = converted;

      // Process chars and build glyph cache
      if (provider.contains("chars")) {
        auto decode_utf8_to_codepoints =
            [](const std::string &s) -> std::vector<uint16_t> {
          std::vector<uint16_t> codes;
          auto it = s.begin();
          while (it != s.end()) {
            try {
              uint32_t code = utf8::next(it, s.end());
              if (code <= 0xFFFF) {
                codes.push_back(static_cast<uint16_t>(code));
              }
            } catch (const utf8::exception &) {
              // Skip invalid UTF-8
              if (it != s.end())
                ++it;
            }
          }
          return codes;
        };

        // Calculate character dimensions
        std::vector<std::vector<uint16_t>> char_rows;
        for (auto &row : provider["chars"]) {
          std::string row_str = row.get<std::string>();
          auto row_codes = decode_utf8_to_codepoints(row_str);
          char_rows.push_back(row_codes);
        }

        if (!char_rows.empty()) {
          int chars_per_row = char_rows[0].size();
          int char_width = bitmapProvider->texture_width / chars_per_row;

          // Build glyph cache with width calculation based on rightmost alpha
          // pixels
          for (int row = 0; row < char_rows.size(); ++row) {
            const auto &row_chars = char_rows[row];
            for (int col = 0; col < row_chars.size(); ++col) {
              uint16_t code = row_chars[col];
              if (code != 0) { // Skip null characters
                BitmapFontProvider::Glyph glyph;
                glyph.u = (col * char_width) /
                          static_cast<float>(bitmapProvider->texture_width);
                glyph.v = (row * bitmapProvider->height) /
                          static_cast<float>(bitmapProvider->texture_height);

                // Calculate actual width based on rightmost column with alpha >
                // 0
                int actual_width = 0;
                int start_x = col * char_width;
                int start_y = row * bitmapProvider->height;

                // Scan from right to left to find rightmost column with alpha
                for (int x = start_x + char_width - 1; x >= start_x; x--) {
                  bool has_alpha = false;
                  for (int y = start_y; y < start_y + bitmapProvider->height &&
                                        y < bitmapProvider->texture_height;
                       y++) {
                    if (x < bitmapProvider->texture_width) {
                      // Access pixel data - surface is RGBA32
                      int pixel_index =
                          (y * bitmapProvider->texture_width + x) * 4;
                      uint8_t *pixels = static_cast<uint8_t *>(surface->pixels);
                      uint8_t alpha = pixels[pixel_index + 3]; // Alpha channel
                      if (alpha > 0) {
                        has_alpha = true;
                        break;
                      }
                    }
                  }
                  if (has_alpha) {
                    actual_width = (x - start_x) + 1;
                    break;
                  }
                }

                // Use at least 1 pixel width if no alpha found, then add 1
                // pixel padding
                glyph.width = static_cast<float>(
                    (actual_width > 0 ? actual_width : 1) + 1);
                bitmapProvider->glyphs[code] = glyph;
              }
            }
          }
        }
      }

      // Now create OpenGL texture and free surface
      glBindTexture(GL_TEXTURE_2D, bitmapProvider->texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmapProvider->texture_width,
                   bitmapProvider->texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                   surface->pixels);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SDL_FreeSurface(surface);
      providers.push_back(std::move(bitmapProvider));
    } else if (type == "space") {
      auto spaceProvider = std::make_unique<SpaceFontProvider>();
      if (provider.contains("advances")) {
        auto decode_utf8 = [](const std::string &s) -> uint32_t {
          if (s.empty())
            return 0;
          try {
            auto it = s.begin();
            return utf8::next(it, s.end());
          } catch (const utf8::exception &) {
            return 0; // invalid UTF-8
          }
        };
        for (auto &[key, val] : provider["advances"].items()) {
          int advance = val.get<int>();
          uint32_t code = decode_utf8(key);
          if (code > 0 && code <= 0xFFFF) {
            spaceProvider->advances[static_cast<uint16_t>(code)] = advance;
          } else {
            throw std::runtime_error(
                "Invalid or unsupported Unicode codepoint in advances: " + key);
          }
        }
      }
      providers.push_back(std::move(spaceProvider));
    } else if (type == "unihex") {
      // TODO: Implement unihex provider
      std::cout << "TODO: Implement unihex provider" << std::endl;
    } else {
      throw std::runtime_error("Unknown font provider type '" + type + "'");
    }
  }
}
