#include "font.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <GLES/gl.h>

using json = nlohmann::json;

ReferenceFontProvider::ReferenceFontProvider(const ResourceLocation &loc)
    : ref(std::make_unique<Font>(loc)) {}

Font::Font(const ResourceLocation &loc) {
  std::ifstream file(loc.to_path("font") + ".json");
  if (!file) {
    throw std::runtime_error("Failed to open font file " + std::string(loc));
  }
  json data = json::parse(file, nullptr, true, false, true);

  for (const auto &provider : data["providers"]) {
    auto type = provider["type"].get<std::string>();
    if (type == "reference") {
      auto refProvider = std::make_unique<ReferenceFontProvider>(
          ResourceLocation(provider["id"].get<std::string>()));
      providers.push_back(std::move(refProvider));
    } else if (type == "bitmap") {
      auto bitmapProvider = std::make_unique<BitmapFontProvider>();
      bitmapProvider->ascent = provider["ascent"].get<int>();
      if (provider.contains("height")) {
        bitmapProvider->height = provider["height"].get<int>();
      } else {
        bitmapProvider->height = 8;
      }
      if (provider.contains("chars")) {
        auto decode_utf8_to_codepoints = [](const std::string& s) -> std::vector<uint16_t> {
          std::vector<uint16_t> codes;
          size_t i = 0;
          while (i < s.size()) {
            uint32_t code = 0;
            unsigned char c = static_cast<unsigned char>(s[i]);
            if (c < 0x80) {
              code = c;
              i += 1;
            } else if (c < 0xE0) {
              if (i + 1 >= s.size()) break;
              code = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i+1]) & 0x3F);
              i += 2;
            } else if (c < 0xF0) {
              if (i + 2 >= s.size()) break;
              code = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+2]) & 0x3F);
              i += 3;
            } else if (c < 0xF8) {
              if (i + 3 >= s.size()) break;
              code = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i+1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[i+2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i+3]) & 0x3F);
              i += 4;
            } else {
              i += 1; // invalid
              continue;
            }
            if (code <= 0xFFFF) {
              codes.push_back(static_cast<uint16_t>(code));
            }
          }
          return codes;
        };
        for (auto& row : provider["chars"]) {
          std::string row_str = row.get<std::string>();
          auto row_codes = decode_utf8_to_codepoints(row_str);
          bitmapProvider->chars.push_back(row_codes);
        }
      }
      std::string file_path = ResourceLocation(provider["file"].get<std::string>()).to_path("textures");
      SDL_Surface* surface = IMG_Load(file_path.c_str());
      if (!surface) {
        throw std::runtime_error("Failed to load texture " + file_path + ": " + IMG_GetError());
      }
      glBindTexture(GL_TEXTURE_2D, bitmapProvider->texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      SDL_FreeSurface(surface);
      providers.push_back(std::move(bitmapProvider));
    } else if (type == "space") {
      auto spaceProvider = std::make_unique<SpaceFontProvider>();
      if (provider.contains("advances")) {
        auto decode_utf8 = [](const std::string& s) -> uint32_t {
          if (s.empty()) return 0;
          unsigned char c = static_cast<unsigned char>(s[0]);
          if (c < 0x80) return c;
          if (c < 0xC0) return 0; // invalid
          if (c < 0xE0) {
            if (s.size() < 2) return 0;
            return ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[1]) & 0x3F);
          }
          if (c < 0xF0) {
            if (s.size() < 3) return 0;
            return ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[2]) & 0x3F);
          }
          if (c < 0xF8) {
            if (s.size() < 4) return 0;
            return ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[3]) & 0x3F);
          }
          return 0; // invalid
        };
        for (auto& [key, val] : provider["advances"].items()) {
          int advance = val.get<int>();
          uint32_t code = decode_utf8(key);
          if (code > 0 && code <= 0xFFFF) {
            spaceProvider->advances[static_cast<uint16_t>(code)] = advance;
          } else {
            throw std::runtime_error("Invalid or unsupported Unicode codepoint in advances: " + key);
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
