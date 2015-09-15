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

int test_cluster() {
  Storage::Cluster cluster;

  if (cluster.getNumHits() != 0 ||
      cluster.getMatchDistance() != 0 ||
      cluster.getPixX() != 0 ||
      cluster.getPixY() != 0 ||
      cluster.getPixErrX() != 0 ||
      cluster.getPixErrY() != 0 ||
      cluster.getPosX() != 0 ||
      cluster.getPosY() != 0 ||
      cluster.getPosZ() != 0 ||
      cluster.getPosErrX() != 0 ||
      cluster.getPosErrY() != 0 ||
      cluster.getPosErrZ() != 0 ||
      cluster.getTiming() != 0 ||
      cluster.getValue() != 0 ||
      cluster.getIndex() != -1 ||
      cluster.fetchTrack() != 0 ||
      cluster.fetchMatchedTrack() != 0 ||
      cluster.fetchPlane() != 0) {
    std::cerr << "Storage::Cluster: default values not as expected" << std::endl;
    return -1;
  }

  cluster.setPix(1,2);
  if (cluster.getPixX() != 1 || cluster.getPixY() != 2) {
    std::cerr << "Storage::Cluster: set/getPix failed" << std::endl;
    return -1;
  }

  cluster.setPixErr(.1, .2);
  if (cluster.getPixErrX() != .1 || cluster.getPixErrY() != .2) {
    std::cerr << "Storage::Cluster: set/getPixErr failed" << std::endl;
    return -1;
  }

  cluster.setPos(.1, .2, .3);
  if (cluster.getPosX() != .1 || 
      cluster.getPosY() != .2 || 
      cluster.getPosZ() != .3) {
    std::cerr << "Storage::Cluster: set/getPos failed" << std::endl;
    return -1;
  }

  cluster.setPosErr(.1, .2, .3);
  if (cluster.getPosErrX() != .1 || 
      cluster.getPosErrY() != .2 || 
      cluster.getPosErrZ() != .3) {
    std::cerr << "Storage::Cluster: set/getPosErr failed" << std::endl;
    return -1;
  }

  cluster.setTiming(.1);
  if (cluster.getTiming() != .1) {
    std::cerr << "Storage::Cluster: set/getTiming failed" << std::endl;
    return -1;
  }

  cluster.setValue(.2);
  if (cluster.getValue() != .2) {
    std::cerr << "Storage::Cluster: set/getValue failed" << std::endl;
    return -1;
  }

  Storage::Hit hit;
  cluster.addHit(hit);
  if (cluster.getNumHits() != 1) {
    std::cerr << "Storage::Cluster: addHit failed" << std::endl;
    return -1;
  }

  if (cluster.getHit(0).fetchCluster() != &cluster) {
    std::cerr << "Storage::Cluster: hit association failed" << std::endl;
    return -1;
  }

  // Try adding the hit to another cluster
  Storage::Cluster cluster2;
  try {
    cluster2.addHit(hit);  // should throw
    std::cerr << "Storage::Cluster: muliply clustered hit" << std::endl;
  } catch (std::runtime_error& e) {
    ;  // caught expected exception (oxymoron?)
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_cluster()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
