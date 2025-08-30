#pragma once
#include "resource_location.hpp"
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include "glxx/texture.hpp"

struct FontProvider {
  virtual ~FontProvider() = default;
};

struct BitmapFontProvider : public FontProvider {
   int ascent;
   std::vector<std::vector<uint16_t>> chars;
   Texture texture;
   int height;
   int texture_width;
   int texture_height;
};

struct SpaceFontProvider : public FontProvider {
  std::unordered_map<uint16_t, int> advances;
};

struct Font;
struct ReferenceFontProvider : public FontProvider {
  std::unique_ptr<Font> ref;

  ReferenceFontProvider(const ResourceLocation &loc);
};

struct Font {
  std::vector<std::unique_ptr<FontProvider>> providers;

  Font(const ResourceLocation &loc);
};
