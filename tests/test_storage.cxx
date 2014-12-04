#include <iostream>
#include <stdexcept>

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
  if (cluster.getPosX() != .1 || cluster.getPosY() != .2 || cluster.getPosZ() != .3) {
    std::cerr << "Storage::Cluster: set/getPos failed" << std::endl;
    return -1;
  }

  cluster.setPosErr(.1, .2, .3);
  if (cluster.getPosErrX() != .1 || cluster.getPosErrY() != .2 || cluster.getPosErrZ() != .3) {
    std::cerr << "Storage::Cluster: set/getPosErr failed" << std::endl;
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

int main() {
  int retval = 0;

  try {
    if ((retval = test_hit()) != 0) return retval;
    if ((retval = test_cluster()) != 0) return retval;
    if ((retval = test_track()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
