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

int test_track() {
  Storage::Track track;

  if (track.getNumClusters() != 0 ||
      track.getNumMatchedClusters() != 0 ||
      track.getOriginX() != 0 ||
      track.getOriginY() != 0 ||
      track.getOriginErrX() != 0 ||
      track.getOriginErrY() != 0 ||
      track.getSlopeX() != 0 ||
      track.getSlopeY() != 0 ||
      track.getSlopeErrX() != 0 ||
      track.getSlopeErrY() != 0 ||
      track.getCovarianceX() != 0 ||
      track.getCovarianceY() != 0 ||
      track.getChi2() != 0 ||
      track.getIndex() != -1) {
    std::cerr << "Storage::Track: default values not as expected" << std::endl;
    return -1;
  }

  track.setOrigin(1,2);
  if (track.getOriginX() != 1 || track.getOriginY() != 2) {
    std::cerr << "Storage::Track: set/getOrigin failed" << std::endl;
    return -1;
  }

  track.setOriginErr(.1, .2);
  if (track.getOriginErrX() != .1 || track.getOriginErrY() != .2) {
    std::cerr << "Storage::Track: set/getOriginErr failed" << std::endl;
    return -1;
  }

  track.setSlope(1,2);
  if (track.getSlopeX() != 1 || track.getSlopeY() != 2) {
    std::cerr << "Storage::Track: set/getSlope failed" << std::endl;
    return -1;
  }

  track.setSlopeErr(.1, .2);
  if (track.getSlopeErrX() != .1 || track.getSlopeErrY() != .2) {
    std::cerr << "Storage::Track: set/getSlopeErr failed" << std::endl;
    return -1;
  }

  track.setCovariance(1,2);
  if (track.getCovarianceX() != 1 || track.getCovarianceY() != 2) {
    std::cerr << "Storage::Track: set/getCovariance failed" << std::endl;
    return -1;
  }

  track.setChi2(.1);
  if (track.getChi2() != .1) {
    std::cerr << "Storage::Track: set/getChi2 failed" << std::endl;
    return -1;
  }

  Storage::Cluster cluster;
  track.addCluster(cluster);
  if (track.getNumClusters() != 1) {
    std::cerr << "Storage::Track: addCluster failed" << std::endl;
    return -1;
  }

  if (track.getCluster(0).fetchTrack() != &track) {
    std::cerr << "Storage::Track: cluster association failed" << std::endl;
    return -1;
  }

  // Try adding the cluster to another track
  Storage::Track track2;
  try {
    track2.addCluster(cluster);
    std::cerr << "Storage::Track: muliply tracked hit" << std::endl;
  } catch (std::runtime_error& e) {
    ;
  }

  return 0;
}

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

int test_storageio() {
  Storage::StorageIO store("tmp.root", Storage::StorageIO::OUTPUT);

  if (store.getNumEvents() != 0 ||
      store.getNumPlanes() != 1 ||
      store.getFileMode() != Storage::StorageIO::OUTPUT ||
      store.getMaskMode() != Storage::StorageIO::REMOVE) {
    std::cerr << "Storage::StorageIO: default values not as expected" << std::endl;
    return -1;
  }

  Storage::Event& event = store.newEvent();

  if (&store.newEvent() != &event) {
    std::cerr << "Storage::StorageIO: no event caching" << std::endl;
    return -1;
  }

  // Make some objects which will be cahed when a new event is requested

  Storage::Hit& hit = event.newHit(0);
  hit.setPix(1, 2);
  hit.setPos(.1, .2, .3);
  hit.setValue(.1);
  hit.setTiming(.2);
  hit.setMasked(true);

  Storage::Cluster& cluster = event.newCluster(0);
  cluster.setPix(.1, .2);
  cluster.setPixErr(.1, .2);
  cluster.setPos(.1, .2, .3);
  cluster.setPosErr(.1, .2, .3);
  cluster.setTiming(.1);
  cluster.setValue(.2);

  Storage::Track& track = event.newTrack();
  track.setOrigin(.1, .2);
  track.setOriginErr(.1, .2);
  track.setSlope(.1, .2);
  track.setSlopeErr(.1, .2);
  track.setCovariance(.1, .2);
  track.setChi2(.1);

  cluster.addHit(hit);
  track.addCluster(cluster);

  // Make a new event and a new hit from the new event. Note that the actual
  // object is the same so we can keep using Event
  store.newEvent();

  // New hit should recycle the old one
  if (&event.newHit(0) != &hit) {
    std::cerr << "Storage::StorageIO: hits not caching" << std::endl;
    return -1;
  }

  // It shoudl be zeroed
  if (hit.getPixX() != 0 ||
      hit.getPixY() != 0 ||
      hit.getPosX() != 0 ||
      hit.getPosY() != 0 ||
      hit.getPosZ() != 0 ||
      hit.getValue() != 0 ||
      hit.getTiming() != 0 ||
      hit.getMasked() != false ||
      hit.fetchCluster() != 0) {
    std::cerr << "Storage::StorageIO: cached hit not zeroed" << std::endl;
    return -1;
  }

  if (&event.newCluster(0) != &cluster) {
    std::cerr << "Storage::StorageIO: clusters not caching" << std::endl;
    return -1;
  }

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
      cluster.getIndex() != 0 ||
      cluster.fetchTrack() != 0 ||
      cluster.fetchMatchedTrack() != 0) {
    std::cerr << "Storage::StorageIO: cached cluster not zeroed" << std::endl;
    return -1;
  }

  if (&event.newTrack() != &track) {
    std::cerr << "Storage::StorageIO: tracks not caching" << std::endl;
    return -1;
  }

  if (track.getNumClusters() != 0 ||
      track.getNumMatchedClusters() != 0 ||
      track.getOriginX() != 0 ||
      track.getOriginY() != 0 ||
      track.getOriginErrX() != 0 ||
      track.getOriginErrY() != 0 ||
      track.getSlopeX() != 0 ||
      track.getSlopeY() != 0 ||
      track.getSlopeErrX() != 0 ||
      track.getSlopeErrY() != 0 ||
      track.getCovarianceX() != 0 ||
      track.getCovarianceY() != 0 ||
      track.getChi2() != 0 ||
      track.getIndex() != 0) {
    std::cerr << "Storage::StorageIO: cahced track not zeroed" << std::endl;
    return -1;
  }

  // Add another hit
  Storage::Hit& hit2 = event.newHit(0);
  // Ensure it is in fact a new hit
  if (&hit2 == &hit) {
    std::cerr << "Storage::StorageIO: cache re-using hits" << std::endl;
    return -1;
  }

  Storage::Cluster& cluster2 = event.newCluster(0);
  if (&cluster2 == &cluster) {
    std::cerr << "Storage::StorageIO: cache re-using clusters" << std::endl;
    return -1;
  }

  Storage::Track& track2 = event.newTrack();
  if (&track2 == &track) {
    std::cerr << "Storage::StorageIO: cache re-using tracks" << std::endl;
    return -1;
  }

  // Load up a new event and ensure both objects were cached. Once more, the
  // event object is the same so our reference is still valid
  store.newEvent();

  // Each call to newHit() should retrieve from the cahce until a the cache
  // is empty
  if (&event.newHit(0) != &hit2 || &event.newHit(0) != &hit) {
    std::cerr << "Storage::StorageIO: hits cache not working back" << std::endl;
    return -1;
  }

  if (&event.newCluster(0) != &cluster2 || &event.newCluster(0) != &cluster) {
    std::cerr << "Storage::StorageIO: clusters cache not working back" << std::endl;
    return -1;
  }

  if (&event.newTrack() != &track2 || &event.newTrack() != &track) {
    std::cerr << "Storage::StorageIO: tracks cache not working back" << std::endl;
    return -1;
  }

  gSystem->Exec("rm -f tmp.root");
  return 0;
}

int test_storageioCache() {
  const unsigned int nplanes = 1;
  Storage::StorageO store("tmp.root", Storage::StorageIO::OUTPUT, nplanes);

  gSystem->Exec("rm -f tmp.root");
  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_hit()) != 0) return retval;
    if ((retval = test_cluster()) != 0) return retval;
    if ((retval = test_track()) != 0) return retval;
    if ((retval = test_event()) != 0) return retval;
    if ((retval = test_plane()) != 0) return retval;
    if ((retval = test_storageio()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
