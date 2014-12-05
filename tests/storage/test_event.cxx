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

int test_event() {
  const size_t nplanes = 2;
  Storage::Event event(nplanes);

  if (event.getNumHits() != 0 ||
      event.getNumClusters() != 0 ||
      event.getNumPlanes() != nplanes ||
      event.getNumTracks() != 0 ||
      event.getTimeStamp() != 0 ||
      event.getFrameNumber() != 0 ||
      event.getTriggerOffset() != 0 ||
      event.getTriggerInfo() != 0 ||
      event.getInvalid() != false) {
    std::cerr << "Storage::Event: default values not as expected" << std::endl;
    return -1;
  }

  event.setTimeStamp(1);
  if (event.getTimeStamp() != 1) {
    std::cerr << "Storage::Event: set/getTimeStamp failed" << std::endl;
    return -1;
  }

  event.setFrameNumber(1);
  if (event.getFrameNumber() != 1) {
    std::cerr << "Storage::Event: set/getFrameNumber failed" << std::endl;
    return -1;
  }

  event.setTriggerOffset(1);
  if (event.getTriggerOffset() != 1) {
    std::cerr << "Storage::Event: set/getTriggerOffset failed" << std::endl;
    return -1;
  }

  event.setTriggerInfo(1);
  if (event.getTriggerInfo() != 1) {
    std::cerr << "Storage::Event: set/getTriggerInfo failed" << std::endl;
    return -1;
  }

  Storage::Hit& hit0 = event.newHit(0);
  Storage::Hit& hit1 = event.newHit(1);

  if (event.getNumHits() != 2 ||
      &event.getHit(0) != &hit0 ||
      &event.getHit(1) != &hit1) {
    std::cerr << "Storage::Event: newHit/get(Num)Hits failed" << std::endl;
    return -1;
  }

  Storage::Cluster& cluster0 = event.newCluster(0);
  Storage::Cluster& cluster1 = event.newCluster(1);

  if (event.getNumClusters() != 2 ||
      &event.getCluster(0) != &cluster0 ||
      &event.getCluster(1) != &cluster1) {
    std::cerr << "Storage::Event: newCluster/get(Num)Clusters failed" << std::endl;
    return -1;
  }

  Storage::Track& track0 = event.newTrack();
  Storage::Track& track1 = event.newTrack();

  if (event.getNumTracks() != 2 || 
      &event.getTrack(0) != &track0 || 
      &event.getTrack(1) != &track1) {
    std::cerr << "Storage::Event: newTrack/get(Num)Tracks failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_event()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
