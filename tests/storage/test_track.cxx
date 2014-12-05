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
    if ((retval = test_track()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
