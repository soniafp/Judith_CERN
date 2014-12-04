#include <iostream>

#include <TApplication.h>

#include "storage/hit.h"

int main(int argc, char** argv) {
  std::cout << "\nStarting Judith\n" << std::endl;

  TApplication app("App", 0, 0);

  std::cout << "\nEnding Judith\n" << std::endl;

  return 0;
}
