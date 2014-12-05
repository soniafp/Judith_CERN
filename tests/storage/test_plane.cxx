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

int test_plane() {
  const size_t nplanes = 2;
  Storage::Event event(nplanes);

  if (event.getPlane(0).getPlaneNum() != 0 || 
      event.getPlane(1).getPlaneNum() != 1) {
    std::cerr << "Storage::Plane: getPlaneNum() failed" << std::endl;
    return -1;
  }

  Storage::Hit& hit0 = event.newHit(0);
  Storage::Hit& hit1 = event.newHit(1);
  Storage::Hit& hit2 = event.newHit(0);

  if (hit0.fetchPlane() != &event.getPlane(0) || 
      hit1.fetchPlane() != &event.getPlane(1) || 
      hit2.fetchPlane() != &event.getPlane(0)) {
    std::cerr << "Storage::Plane: hit plane association failed" << std::endl;
    return -1;
  }

  Storage::Cluster& cluster0 = event.newCluster(0);
  Storage::Cluster& cluster1 = event.newCluster(1);
  Storage::Cluster& cluster2 = event.newCluster(0);

  if (cluster0.fetchPlane() != &event.getPlane(0) || 
      cluster1.fetchPlane() != &event.getPlane(1) || 
      cluster2.fetchPlane() != &event.getPlane(0)) {
    std::cerr << "Storage::Plane: cluster plane association failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_plane()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
