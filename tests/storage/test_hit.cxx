#include <iostream>
#include <stdexcept>

#include <TSystem.h>

#include "storage/storageo.h"
#include "storage/storagei.h"
#include "storage/storageio.h"
#include "storage/event.h"
#include "storage/track.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "storage/hit.h"

int test_hit() {
  Storage::Hit hit;

  if (hit.getPixX() != 0 ||
      hit.getPixY() != 0 ||
      hit.getPosX() != 0 ||
      hit.getPosY() != 0 ||
      hit.getPosZ() != 0 ||
      hit.getValue() != 0 ||
      hit.getTiming() != 0 ||
      hit.getMasked() != false ||
      hit.fetchCluster() != 0 ||
      hit.fetchPlane() != 0) {
    std::cerr << "Storage::Hit: default values not as expected" << std::endl;
    return -1;
  }

  hit.setPix(1,2);
  if (hit.getPixX() != 1 || hit.getPixY() != 2) {
    std::cerr << "Storage::Hit: set/getPix failed" << std::endl;
    return -1;
  }

  hit.setPos(.1, .2, .3);
  if (hit.getPosX() != .1 || hit.getPosY() != .2 || hit.getPosZ() != .3) {
    std::cerr << "Storage::Hit: set/getPos failed" << std::endl;
    return -1;
  }

  hit.setValue(.1);
  if (hit.getValue() != .1) {
    std::cerr << "Storage::Hit: set/getValue failed" << std::endl;
    return -1;
  }

  hit.setTiming(.1);
  if (hit.getTiming() != .1) {
    std::cerr << "Storage::Hit: set/getTiming failed" << std::endl;
    return -1;
  }

  hit.setMasked(true);
  if (hit.getMasked() != true) {
    std::cerr << "Storage::Hit: set/getMask failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_hit()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
