#pragma once
#include "resource_location.hpp"
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include "glxx/texture.hpp"

struct FontProvider {
  virtual ~FontProvider() = default;
  virtual bool handles_character(uint16_t code) const = 0;
};

struct BitmapFontProvider : public FontProvider {
  struct Glyph {
    float u;
    float v;
    float width;
  };

  int ascent;
  int height;
  std::unordered_map<uint16_t, Glyph> glyphs;
  Texture texture;
  int texture_width;
  int texture_height;

  bool handles_character(uint16_t code) const override {
    return glyphs.find(code) != glyphs.end();
  }
};

struct SpaceFontProvider : public FontProvider {
  std::unordered_map<uint16_t, int> advances;

  bool handles_character(uint16_t code) const override {
    return advances.find(code) != advances.end();
  }
};

struct Font;
struct ReferenceFontProvider : public FontProvider {
  std::unique_ptr<Font> ref;

  ReferenceFontProvider(const ResourceLocation &loc);
  
  bool handles_character(uint16_t code) const override;
};

struct Font {
  std::vector<std::unique_ptr<FontProvider>> providers;

  Font(const ResourceLocation &loc);
};
