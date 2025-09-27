#include "texture.hpp"

Texture::Texture() {
  glGenTextures(1, &texture_id);
}

Texture::~Texture() {
  if (texture_id != 0) {
    glDeleteTextures(1, &texture_id);
    texture_id = 0;
  }
}

void Texture::bind() const {
  glBindTexture(GL_TEXTURE_2D, texture_id);
}
