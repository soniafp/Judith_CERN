#include <iostream>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>

#include <TApplication.h>

#include "options.h"
#include "storage/storagei.h"
#include "storage/storageo.h"

void printHelp() {
  printf("usage: judith <command> [<args>]\n");

  printf("\nArguments:\n");
  printf("  %2s %-15s %s\n", "-h", "--help", "Display this information");
  printf("  %2s %-15s %s\n", "-i", "--input", "Path to input file");
  printf("  %2s %-15s %s\n", "-o", "--output", "Path to input file");
  printf("  %2s %-15s %s\n", "-s", "--settings", "Path to settings file (default: configs/settings.cfg)");
  printf("  %2s %-15s %s\n", "-r", "--results", "Path to results file");

  printf("\nCommands:\n");
  printf("  %-15s %s\n", "process", "Generate clusters and tracks from the given input");
  std::cout << std::endl;
}

void fillBranchMasks(
    const Options& options,
    std::set<std::string> hitBranchesOff,
    std::set<std::string> clusterBranchesOff,
    std::set<std::string> trackBranchesOff,
    std::set<std::string> eventInfoBranchesOff) {
  // Setup an enum to loop over the sets to fill
  enum Trees { HIT, CLUSTER, TRACK, EVENTINFO };

  // Loop over the sets to fill
  for (int tree = HIT; tree <= EVENTINFO; tree++) {
    std::string key;  // options key listing branches
    std::set<std::string>* branches;  // pointer to the set to fill
    switch (tree) {
    case HIT:
      key = "hit-branch-off";
      branches = &hitBranchesOff;
      break;
    case CLUSTER:
      key = "cluster-branch-off";
      branches = &clusterBranchesOff;
      break;
    case TRACK:
      key = "track-branch-off";
      branches = &trackBranchesOff;
      break;
    case EVENTINFO:
      key = "eventInfo-branch-off";
      branches = &eventInfoBranchesOff;
      break;
    }

    // Get the branch names to be turned off for the given key
    const Options::Values& values = options.getValues(key);
    for (Options::Values::const_iterator it = values.begin();
        it != values.end(); it++)
      if (options.evalBoolArg(*it)) branches->insert(*it);
  }
}

void generatePlanesMask(
    const Options& options,
    std::vector<bool>& mask) {
  if (!options.hasArg("mask-planes")) return;
  // The first value is the number of planes
  const int nplanes = strToInt(options.getValue("mask-planes"));
  mask.assign(nplanes, false);
  // The remaining values are indices of planes to mask
  const Options::Values& values = options.getValues("mask-planes");
  for (size_t ival = 1; ival < values.size(); ival++) {
    const int iplane = strToInt(values[ival]);
    mask[iplane] = true;
  }
}

int main(int argc, const char** argv) {
  std::cout << "\nStarting Judith\n" << std::endl;

  TApplication app("App", 0, 0);

  Options options;
  options.defineShort('i', "input");
  options.defineShort('o', "output");
  options.defineShort('s', "settings");
  options.defineShort('r', "results");

  // Parse options
  try {
    // First command line options
    options.parseArgs(argc, argv);
    // Check if help is requested and stop execution if so
    if (options.hasArg("help")) {
      printHelp();
      return 0;
    }
    // Default settings path (overwritten if provided in command line arguments)
    options.addPair("settings", "configs/settings.cfg");
    // Parse settings file
    options.parseFile(options.getValue("settings"));
  }
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }
  
  // If no additional arguments are given, or the first is an argument, then
  // no command was provided
  if (argc < 2 || argv[1][0] == '-') {
    std::cerr << "ERROR: no command provided" << std::endl;
    printHelp();
    return -1;
  }

  // Get lists of branches to turn off from configuration
  std::set<std::string> hitBranchesOff;
  std::set<std::string> clusterBranchesOff;
  std::set<std::string> trackBranchesOff;
  std::set<std::string> eventInfoBranchesOff;
  fillBranchMasks(
      options,
      hitBranchesOff,
      clusterBranchesOff,
      trackBranchesOff,
      eventInfoBranchesOff);

  // Get the list of masked planes
  std::vector<bool> planeMask;
  generatePlanesMask(options, planeMask);

  const std::string command = argv[1];

  /////////////////////////////////////////////////////////////////////////////
  // Processing

  if (command == "process") {
    if (!options.hasArg("input") || !options.hasArg("output")) {
      std::cerr << "ERROR: process requires and input and output arguments"
          << std::endl;
      return -1;
    }

    // List of TTree branches to turn off on read back
    std::set<std::string> inHitsOff;
    inHitsOff.insert("hitPosX");
    inHitsOff.insert("hitPosY");
    inHitsOff.insert("hitPosZ");

    Storage::StorageI input(
        options.getValue("input"),
        // Don't read back clusters and tracks since they are not used
        Storage::StorageIO::CLUSTERS | Storage::StorageIO::TRACKS,
        // Planes to mask: either a null pointer or the vector if filled
        planeMask.empty() ? 0 : &planeMask,
        // Don't read hit global positions since they will be re-generated
        &inHitsOff);

    int outTreeMask = 0;

    if (!options.evalBoolArg("process-clusters"))
      outTreeMask |= Storage::StorageIO::CLUSTERS;
    if (!options.evalBoolArg("process-tracks"))
      outTreeMask |= Storage::StorageIO::TRACKS;

    Storage::StorageO output(
        options.getValue("output"),
        input.getNumPlanes(),
        outTreeMask,
        &hitBranchesOff,
        &clusterBranchesOff,
        &trackBranchesOff,
        &eventInfoBranchesOff);
  }

  else {
    std::cerr << "ERROR: unknown command " << command << std::endl;
    printHelp();
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
