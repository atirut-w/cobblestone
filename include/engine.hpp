#pragma once
#include "singleton.hpp"

class Engine : public Singleton<Engine> {
public:
  int start(int argc, char *argv[]);
};
