#pragma once
#include "singleton.hpp"

class Engine : public Singleton<Engine> {
public:
  int main(int argc, char *argv[]);
};
