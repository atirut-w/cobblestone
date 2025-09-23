#pragma once
#include <GL/gl.h>

struct Texture {
  GLuint id;

  Texture() {
    glGenTextures(1, &id);
  }

  ~Texture() {
    glDeleteTextures(1, &id);
  }

  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  Texture(Texture &&other) noexcept : id(other.id) {
    other.id = 0;
  }

  Texture &operator=(Texture &&other) noexcept {
    if (this != &other) {
      glDeleteTextures(1, &id);
      id = other.id;
      other.id = 0;
    }
    return *this;
  }

  operator GLuint() const {
    return id;
  }
};
