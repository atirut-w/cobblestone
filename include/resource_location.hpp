#pragma once
#include <algorithm>
#include <stdexcept>
#include <string>

struct ResourceLocation {
  std::string ns;
  std::string path;

  ResourceLocation(const std::string &str) {
    if (std::count(str.begin(), str.end(), ':') > 1) {
      throw std::invalid_argument("Invalid resource location");
    }
    auto pos = str.find(':');
    if (pos != std::string::npos) {
      if (std::count(str.begin(), str.begin() + pos, '/') > 0) {
        throw std::invalid_argument("Illegal character in namespace");
      }
      ns = str.substr(0, pos);
      path = str.substr(pos + 1);
    } else {
      ns = "minecraft";
      path = str;
    }
  }
};
