#pragma once
#include "SDL_image.h"
#include "SDL_opengl.h"

class Texture {
  GLuint texture_id = 0;

public:
  Texture();
  Texture(const SDL_Surface &surface);
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;
  ~Texture();

  void bind() const;
};
