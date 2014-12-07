#include <iostream>
#include <stdio>
#include <stdexcept>
#include <string>
#include <vector>

#include <TApplication.h>

#include "configuration/options.h"
#include "storage/hit.h"



void printHelp() {
  printf("usage: judith [<args>]\n");

  printf("\nArguments:\n");
  printf("  %2s %-15s %s\n", "-h", "--help", "Display this information");
  printf("  %2s %-15s %s\n", "-s", "--settings", "Path to settings file (default: data/fbusettings.cfg)");
  printf("  %2s %-15s %s\n", "-i", "--input", "Path to input file");
  printf("  %2s %-15s %s\n", "-o", "--output", "Path to input file");
  std::cout << std::endl;
}

int main(int argc, char** argv) {
  std::cout << "\nStarting Judith\n" << std::endl;

  TApplication app("App", 0, 0);

  Configuration::Options options;
  options.defineShort('i', "input");
  options.defineShort('o', "output");
  options.defineShort('s', "settings");
  options.defineShort('r', "results");
  options.defineShort('p', "process");

  // Parse options
  try {
    // First command line options
    options.parseArgs(argc, argv);
    // Check if help is requested and stop execution if so
    if (config.hasArg("help")) {
      printHelp();
      return 0;
    }
    // Default settings path (overwritten if provided in command line arguments)
    options.addPair("settings", "configs/settings.txt");
    // Parse settings file
    options.parseFile(options.getValue("settings");
  }
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }
  
  // Possible requests
  // mask : only hits
  // cluster : only hits
  // track : hits and clusters
  // align : hits and clusters
  // sync : everything

  std::cout << "\nEnding Judith\n" << std::endl;

  return 0;
}
