#include "texture.hpp"
#include <format>
#include <stdexcept>

Texture::Texture() { glGenTextures(1, &texture_id); }

Texture::Texture(const SDL_Surface &surface) : Texture() {
  glBindTexture(GL_TEXTURE_2D, texture_id);

  SDL_Surface *converted = SDL_ConvertSurfaceFormat(
      const_cast<SDL_Surface *>(&surface), SDL_PIXELFORMAT_RGBA32, 0);
  if (!converted) {
    throw std::runtime_error(
        std::format("Failed to convert surface format: {}", SDL_GetError()));
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, converted->w, converted->h, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);
  SDL_FreeSurface(converted);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::~Texture() {
  if (texture_id != 0) {
    glDeleteTextures(1, &texture_id);
    texture_id = 0;
  }
}

void Texture::bind() const { glBindTexture(GL_TEXTURE_2D, texture_id); }
