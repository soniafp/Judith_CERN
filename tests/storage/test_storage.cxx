#include <iostream>
#include <stdexcept>
#include <cmath>
#include <set>

#include <TSystem.h>

#include "storage/storageo.h"
#include "storage/storagei.h"
#include "storage/storageio.h"
#include "storage/event.h"
#include "storage/track.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "storage/hit.h"

#define NPLANES 1
#define NEVENTS 2

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
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

int test_storageioWrite() {
  Storage::StorageO store("tmp.root", NPLANES);

  // Make some number of events
  for (size_t n = 0; n < NEVENTS; n++) {
    Storage::Event& event = store.newEvent();

    event.setTimeStamp(n);
    event.setFrameNumber(n+1);
    event.setTriggerOffset(n+2);
    event.setTriggerInfo(n+3);

    // Alternate the plane populated at each event
    Storage::Hit& hit = event.newHit(n%NPLANES);
    // Scale the values by the event
    hit.setPix(1*n+1, 2*n+1);
    hit.setPos(.1*n+1, .2*n+1, .3*n+1);
    hit.setTiming(1*n+1);
    hit.setValue(2*n+1);
    hit.setMasked(true);

    Storage::Cluster& cluster = event.newCluster(n%NPLANES);
    cluster.setPix(.1*n+1, .2*n+1);
    cluster.setPixErr(.1*n+1, .2*n+1);
    cluster.setPos(.1*n+1, .2*n+1, .3*n+1);
    cluster.setPosErr(.1*n+1, .2*n+1, .3*n+1);
    cluster.setTiming(.1*n+1);
    cluster.setValue(.2*n+1);

    Storage::Track& track = event.newTrack();
    track.setOrigin(.1*n+1, .2*n+1);
    track.setOriginErr(.1*n+1, .2*n+1);
    track.setSlope(.1*n+1, .2*n+1);
    track.setSlopeErr(.1*n+1, .2*n+1);
    track.setCovariance(.1*n+1, .2*n+1);
    track.setChi2(.1*n+1);

    cluster.addHit(hit);
    track.addCluster(cluster);

    store.writeEvent(event);
  }

  return 0;
}

int test_storageioRead() {
  Storage::StorageI store("tmp.root");

  if (NEVENTS != store.getNumEvents()) {
    std::cerr << "Storage::StorageI: incorrect number of events" << std::endl;
    return -1;
  }

  if (NPLANES != store.getNumPlanes()) {
    std::cerr << "Storage::StorageI: incorrect number of planes" << std::endl;
    return -1;
  }

  // Read back the events in the file
  for (Int_t n = 0; n < store.getNumEvents(); n++) {
    Storage::Event& event = store.readEvent(n);

    if (event.getTimeStamp() != (unsigned int)n ||
        event.getFrameNumber() != (unsigned int)n+1 ||
        event.getTriggerOffset() != n+2 ||
        event.getTriggerInfo() != n+3) {
      std::cerr << "StorageI::StorageI: event read back incorrect" << std::endl;
      return -1;
    }

    if (event.getNumTracks() != 1) {
      std::cerr << "Storage::StorageI: incorrect number of tracks" << std::endl;
      return -1;
    }

    Storage::Track& track = event.getTrack(0);
    if (!approxEqual(track.getOriginX(), .1*n+1) ||
        !approxEqual(track.getOriginY(), .2*n+1) ||
        !approxEqual(track.getOriginErrX(), .1*n+1) ||
        !approxEqual(track.getOriginErrY(), .2*n+1) ||
        !approxEqual(track.getSlopeX(), .1*n+1) ||
        !approxEqual(track.getSlopeY(), .2*n+1) ||
        !approxEqual(track.getSlopeErrX(), .1*n+1) ||
        !approxEqual(track.getSlopeErrY(), .2*n+1) ||
        !approxEqual(track.getCovarianceX(), .1*n+1) ||
        !approxEqual(track.getCovarianceY(), .2*n+1) ||
        !approxEqual(track.getChi2(), .1*n+1)) {
      std::cerr << "Storage::StorageI: track read back incorrect" << std::endl;
      return -1;
    }

    if (event.getNumClusters() != 1) {
      std::cerr << "Storage::StorageI: incorrect number of clusters" << std::endl;
      return -1;
    }

    Storage::Cluster& cluster = event.getCluster(0);
    if (!approxEqual(cluster.getPixX(), .1*n+1) ||
        !approxEqual(cluster.getPixY(), .2*n+1) ||
        !approxEqual(cluster.getPixErrX(), .1*n+1) ||
        !approxEqual(cluster.getPixErrY(), .2*n+1) ||
        !approxEqual(cluster.getPosX(), .1*n+1) ||
        !approxEqual(cluster.getPosY(), .2*n+1) ||
        !approxEqual(cluster.getPosZ(), .3*n+1) ||
        !approxEqual(cluster.getPosErrX(), .1*n+1) ||
        !approxEqual(cluster.getPosErrY(), .2*n+1) ||
        !approxEqual(cluster.getPosErrZ(), .3*n+1) ||
        !approxEqual(cluster.getTiming(), .1*n+1) ||
        !approxEqual(cluster.getValue(), .2*n+1)) {
      std::cerr << "Storage::StorageI: cluster read back incorrect" << std::endl;
      return -1;
    }

    if (event.getNumClusters() != 1) {
      std::cerr << "Storage::StorageI: incorrect number of clusters" << std::endl;
      return -1;
    }

    Storage::Hit& hit = event.getHit(0);
    if (!approxEqual(hit.getPixX(), 1*n+1) ||
        !approxEqual(hit.getPixY(), 2*n+1) ||
        !approxEqual(hit.getPosX(), .1*n+1) ||
        !approxEqual(hit.getPosY(), .2*n+1) ||
        !approxEqual(hit.getPosZ(), .3*n+1) ||
        hit.getTiming() != 1*n+1 ||
        hit.getValue() != 2*n+1) {
      std::cerr << "Storage::StorageI: hit read back incorrect" << std::endl;
      return -1;
    }
  }

  return 0;
}

int test_storageioReadMasking() {
  {  // Hit masking
    Storage::StorageI store(
        "tmp.root",
        Storage::StorageIO::HITS);

    // Read back the events in the file
    for (Int_t n = 0; n < store.getNumEvents(); n++) {
      Storage::Event& event = store.readEvent(n);
      if (event.getNumHits() > 0) {
        std::cerr << "Storage::StorageI: hit tree mask failed" << std::endl;
        return -1;
      }
    }
  }

  {  // Cluster masking
    Storage::StorageI store(
        "tmp.root",
        Storage::StorageIO::CLUSTERS);

    for (Int_t n = 0; n < store.getNumEvents(); n++) {
      Storage::Event& event = store.readEvent(n);
      if (event.getNumClusters() > 0) {
        std::cerr << "Storage::StorageI: cluster tree mask failed" << std::endl;
        return -1;
      }
    }
  }

  {  // Track masking
    Storage::StorageI store(
        "tmp.root",
        Storage::StorageIO::TRACKS);

    for (Int_t n = 0; n < store.getNumEvents(); n++) {
      Storage::Event& event = store.readEvent(n);
      if (event.getNumTracks() > 0) {
        std::cerr << "Storage::StorageI: track tree mask failed" << std::endl;
        return -1;
      }
    }
  }

  {  // Event info masking
    Storage::StorageI store(
        "tmp.root",
        Storage::StorageIO::EVENTINFO);

    for (Int_t n = 0; n < store.getNumEvents(); n++) {
      Storage::Event& event = store.readEvent(n);
      if (event.getTimeStamp() != 0 ||
          event.getFrameNumber() != 0 ||
          event.getTriggerOffset() != 0 ||
          event.getTriggerInfo() != 0) {
        std::cerr << "Storage::StorageI: event info tree mask failed" << std::endl;
        return -1;
      }
    }
  }

  {  // Position, value and timing masking
    std::set<std::string> hitsBranchMask;
    hitsBranchMask.insert("PixX");
    hitsBranchMask.insert("PixY");
    hitsBranchMask.insert("PosX");
    hitsBranchMask.insert("PosY");
    hitsBranchMask.insert("PosZ");
    hitsBranchMask.insert("Value");
    hitsBranchMask.insert("Timing");

    std::set<std::string> clustersBranchMask;
    clustersBranchMask.insert("PixX");
    clustersBranchMask.insert("PixY");
    clustersBranchMask.insert("PixErrX");
    clustersBranchMask.insert("PixErrY");
    clustersBranchMask.insert("PosX");
    clustersBranchMask.insert("PosY");
    clustersBranchMask.insert("PosZ");
    clustersBranchMask.insert("PosErrX");
    clustersBranchMask.insert("PosErrY");
    clustersBranchMask.insert("PosErrZ");
    clustersBranchMask.insert("Value");
    clustersBranchMask.insert("Timing");

    std::set<std::string> tracksBranchMask;
    tracksBranchMask.insert("SlopeX");
    tracksBranchMask.insert("SlopeY");
    tracksBranchMask.insert("SlopeErrX");
    tracksBranchMask.insert("SlopeErrY");
    tracksBranchMask.insert("OriginX");
    tracksBranchMask.insert("OriginY");
    tracksBranchMask.insert("OriginErrX");
    tracksBranchMask.insert("OriginErrY");
    tracksBranchMask.insert("CovarianceX");
    tracksBranchMask.insert("CovarianceY");
    tracksBranchMask.insert("Chi2");

    std::set<std::string> eventInfoBranchMask;
    eventInfoBranchMask.insert("TimeStamp");
    eventInfoBranchMask.insert("FrameNumber");
    eventInfoBranchMask.insert("TriggerOffset");
    eventInfoBranchMask.insert("TriggerInfo");
    eventInfoBranchMask.insert("Invalid");

    Storage::StorageI store(
        "tmp.root",
        Storage::StorageIO::NONE, // No tree mask
        0, // No plane mask
        &hitsBranchMask,
        &clustersBranchMask,
        &tracksBranchMask,
        &eventInfoBranchMask);

    for (Int_t n = 0; n < store.getNumEvents(); n++) {
      Storage::Event& event = store.readEvent(n);
      Storage::Hit& hit = event.getHit(0);
      Storage::Cluster& cluster = event.getCluster(0);
      Storage::Track& track = event.getTrack(0);
      if (hit.getPixX() != 0 ||
          hit.getPixY() != 0 ||
          hit.getPosX() != 0 ||
          hit.getPosY() != 0 ||
          hit.getPosZ() != 0 ||
          hit.getValue() != 0 ||
          hit.getTiming() != 0 ||
          cluster.getPixX() != 0 ||
          cluster.getPixY() != 0 ||
          cluster.getPixErrX() != 0 ||
          cluster.getPixErrY() != 0 ||
          cluster.getPosX() != 0 ||
          cluster.getPosY() != 0 ||
          cluster.getPosZ() != 0 ||
          cluster.getValue() != 0 ||
          cluster.getTiming() != 0 ||
          track.getSlopeX() != 0 ||
          track.getSlopeY() != 0 ||
          track.getSlopeErrX() != 0 ||
          track.getSlopeErrY() != 0 ||
          track.getOriginX() != 0 ||
          track.getOriginY() != 0 ||
          track.getOriginErrX() != 0 ||
          track.getOriginErrY() != 0 ||
          track.getCovarianceX() != 0 ||
          track.getCovarianceY() != 0 ||
          event.getTimeStamp() != 0 ||
          event.getFrameNumber() != 0 ||
          event.getTriggerOffset() != 0 ||
          event.getTriggerInfo() != 0 ||
          event.getInvalid() != false) {
        std::cerr << "Storage::StorageI: branch mask failed" << std::endl;
        return -1;
      }
    }
  }

  return 0;
}

// TODO test masking on write

int main() {
  int retval = 0;

  try {
    if ((retval = test_storageio()) != 0) return retval;
    if ((retval = test_storageioWrite()) != 0) return retval;
    if ((retval = test_storageioRead()) != 0) return retval;
    if ((retval = test_storageioReadMasking()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  // Remove file on success, otherwise keep it so it can be consulted
  gSystem->Exec("rm -f tmp.root");

  return 0;
}
