#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <set>

#include <TFile.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TBranch.h>

#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/track.h"
#include "storage/event.h"
#include "storage/storageio.h"
#include "storage/storagei.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

namespace Storage {

StorageI::StorageI(
    const std::string& filePath,
    int treeMask,
    const std::vector<bool>* planeMask,
    const std::set<std::string>* hitsBranchesOff,
    const std::set<std::string>* clustersBranchesOff,
    const std::set<std::string>* tracksBranchesOff,
    const std::set<std::string>* eventInfoBranchesOff) :
    // Initialize base with 0 planes and count them as they are read in
    StorageIO(filePath, INPUT, 0) {

  // Invert the mask to not have to check !
  treeMask = ~treeMask;

  // Copy any/all given branch masks
  if (hitsBranchesOff) m_hitsBranchesOff = *hitsBranchesOff;
  if (clustersBranchesOff) m_clustersBranchesOff = *clustersBranchesOff;
  if (tracksBranchesOff) m_tracksBranchesOff = *tracksBranchesOff;
  if (eventInfoBranchesOff) m_eventInfoBranchesOff = *eventInfoBranchesOff;

  // Keep track of the number of planes read from the file (not the same as the
  // number stored in `m_numPlanes` since some might be masked)
  size_t planeCount = 0;

  // Read all planes from the file
  while (true) {
    // Try to get this plane's directory
    std::stringstream ss;
    ss << "Plane" << planeCount;  // Directories are named PlaneX
    TDirectory* dir = 0;
    m_file.GetObject(ss.str().c_str(), dir);

    // When no more plane directories are found, stop
    if (!dir) break;

    planeCount += 1;

    if (planeMask && planeCount > planeMask->size())
      throw std::runtime_error(
          "StorageI::StorageI: plane mask is too small");

    // Skip loading this plane from file if masked
    if (planeMask && planeMask->at(planeCount-1))
      continue;

    m_numPlanes += 1;

    TTree* hits = 0;
    // Try to load the tree if hits are enabled
    if (treeMask & HITS)
      m_file.GetObject((ss.str()+"/Hits").c_str(), hits);
    // Check that a hits tree was loaded
    if (hits) {
      // Add this tree to the current plane
      m_hitsTrees.push_back(hits);
      // Associate the tree's branch to local memory
      m_hitsBranchesOff.insert("NHits");
      hits->SetBranchAddress("NHits", &numHits);
      // Check if the branch is flagged off
      if (!isHitsBranchOff("PixX")) {
        // Check if the branch is in the file, and flag off if not
        if (!hits->GetBranch("PixX")) m_hitsBranchesOff.insert("PixX");
        // Associate the branch to local memory
        else hits->SetBranchAddress("PixX", hitPixX);
      }
      if (!isHitsBranchOff("PixY")) {
        if (!hits->GetBranch("PixY")) m_hitsBranchesOff.insert("PixY");
        else hits->SetBranchAddress("PixY", hitPixY);
      }
      if (!isHitsBranchOff("PosX")) {
        if (!hits->GetBranch("PosX")) m_hitsBranchesOff.insert("PosX");
        else hits->SetBranchAddress("PosX", hitPosX);
      }
      if (!isHitsBranchOff("PosY")) {
        if (!hits->GetBranch("PosY")) m_hitsBranchesOff.insert("PosY");
        else hits->SetBranchAddress("PosY", hitPosY);
      }
      if (!isHitsBranchOff("PosZ")) {
        if (!hits->GetBranch("PosZ")) m_hitsBranchesOff.insert("PosZ");
        else hits->SetBranchAddress("PosZ", hitPosZ);
      }
      if (!isHitsBranchOff("Value")) {
        if (!hits->GetBranch("Value")) m_hitsBranchesOff.insert("Value");
        else hits->SetBranchAddress("Value", hitValue);
      }
      if (!isHitsBranchOff("Timing")) {
        if (!hits->GetBranch("Timing")) m_hitsBranchesOff.insert("Timing");
        else hits->SetBranchAddress("Timing", hitTiming);
      }
      // Also check if cluster tree is masked before enabling association branch
      if (!isHitsBranchOff("InCluster") && (treeMask & CLUSTERS)) {
        if (!hits->GetBranch("InCluster")) m_hitsBranchesOff.insert("InCluster");
        else hits->SetBranchAddress("InCluster", hitInCluster);
      }
    }

    TTree* clusters = 0;
    if (treeMask & CLUSTERS)
      m_file.GetObject((ss.str()+"/Clusters").c_str(), clusters);
    if (clusters) {
      m_clustersTrees.push_back(clusters);
      clusters->SetBranchAddress("NClusters", &numClusters);
      if (!isClustersBranchOff("PixX")) {
        if (!clusters->GetBranch("PixX")) m_clustersBranchesOff.insert("PixX");
        else clusters->SetBranchAddress("PixX", clusterPixX);
      }
      if (!isClustersBranchOff("PixY")) {
        if (!clusters->GetBranch("PixY")) m_clustersBranchesOff.insert("PixY");
        else clusters->SetBranchAddress("PixY", clusterPixY);
      }
      if (!isClustersBranchOff("PixY")) {
        if (!clusters->GetBranch("PixY")) m_clustersBranchesOff.insert("PixY");
        else clusters->SetBranchAddress("PixY", clusterPixY);
      }
      if (!isClustersBranchOff("PixErrX")) {
        if (!clusters->GetBranch("PixErrX")) m_clustersBranchesOff.insert("PixErrX");
        else clusters->SetBranchAddress("PixErrX", clusterPixErrX);
      }
      if (!isClustersBranchOff("PixErrY")) {
        if (!clusters->GetBranch("PixErrY")) m_clustersBranchesOff.insert("PixErrY");
        else clusters->SetBranchAddress("PixErrY", clusterPixErrY);
      }
      if (!isClustersBranchOff("PosX")) {
        if (!clusters->GetBranch("PosX")) m_clustersBranchesOff.insert("PosX");
        else clusters->SetBranchAddress("PosX", clusterPosX);
      }
      if (!isClustersBranchOff("PosY")) {
        if (!clusters->GetBranch("PosY")) m_clustersBranchesOff.insert("PosY");
        else clusters->SetBranchAddress("PosY", clusterPosY);
      }
      if (!isClustersBranchOff("PosZ")) {
        if (!clusters->GetBranch("PosZ")) m_clustersBranchesOff.insert("PosZ");
        else clusters->SetBranchAddress("PosZ", clusterPosZ);
      }
      if (!isClustersBranchOff("PosErrX")) {
        if (!clusters->GetBranch("PosErrX")) m_clustersBranchesOff.insert("PosErrX");
        else clusters->SetBranchAddress("PosErrX", clusterPosErrX);
      }
      if (!isClustersBranchOff("PosErrY")) {
        if (!clusters->GetBranch("PosErrY")) m_clustersBranchesOff.insert("PosErrY");
        else clusters->SetBranchAddress("PosErrY", clusterPosErrY);
      }
      if (!isClustersBranchOff("PosErrZ")) {
        if (!clusters->GetBranch("PosErrZ")) m_clustersBranchesOff.insert("PosErrZ");
        else clusters->SetBranchAddress("PosErrZ", clusterPosErrZ);
      }
      if (!isClustersBranchOff("Value")) {
        if (!clusters->GetBranch("Value")) m_clustersBranchesOff.insert("Value");
        else clusters->SetBranchAddress("Value", clusterValue);
      }
      if (!isClustersBranchOff("Timing")) {
        if (!clusters->GetBranch("Timing")) m_clustersBranchesOff.insert("Timing");
        else clusters->SetBranchAddress("Timing", clusterTiming);
      }
      if (!isClustersBranchOff("InTrack") && (treeMask & TRACKS)) {
        if (!clusters->GetBranch("InTrack")) m_clustersBranchesOff.insert("InTrack");
        else clusters->SetBranchAddress("InTrack", clusterInTrack);
      }
    }
  }  // Loop over planes

  if (m_numPlanes == 0)
    throw std::runtime_error(
        "StorageI::StorageI: zero planes read from file");

  if (!m_hitsTrees.empty() && m_hitsTrees.size() != m_numPlanes)
    throw std::runtime_error(
        "StorageI::StorageI: hits trees number does not match planes");

  if (!m_clustersTrees.empty() && m_clustersTrees.size() != m_numPlanes)
    throw std::runtime_error(
        "StorageI::StorageI: clusters trees number does not match planes");

  // Check if clusters are given, hits are given, but hits aren't associated
  // with clusters
  if (!m_clustersTrees.empty() &&
      !m_hitsTrees.empty() &&
      isHitsBranchOff("InCluster"))
    throw std::runtime_error(
        "StorageI::StorageI: clusters are provided without hit associations");

  if (treeMask & EVENTINFO)
    m_file.GetObject("Event", m_eventInfoTree);
  if (m_eventInfoTree) {
    if (!isEventInfoBranchOff("TimeStamp")) {
      if (!m_eventInfoTree->GetBranch("TimeStamp")) m_eventInfoBranchesOff.insert("TimeStamp");
      else m_eventInfoTree->SetBranchAddress("TimeStamp", &timeStamp);
    }
    if (!isEventInfoBranchOff("FrameNumber")) {
      if (!m_eventInfoTree->GetBranch("FrameNumber")) m_eventInfoBranchesOff.insert("FrameNumber");
      else m_eventInfoTree->SetBranchAddress("FrameNumber", &frameNumber);
    }
    if (!isEventInfoBranchOff("TriggerOffset")) {
      if (!m_eventInfoTree->GetBranch("TriggerOffset")) m_eventInfoBranchesOff.insert("TriggerOffset");
      else m_eventInfoTree->SetBranchAddress("TriggerOffset", &triggerOffset);
    }
    if (!isEventInfoBranchOff("TriggerInfo")) {
      if (!m_eventInfoTree->GetBranch("TriggerInfo")) m_eventInfoBranchesOff.insert("TriggerInfo");
      else m_eventInfoTree->SetBranchAddress("TriggerInfo", &triggerInfo);
    }
    if (!isEventInfoBranchOff("Invalid")) {
      if (!m_eventInfoTree->GetBranch("Invalid")) m_eventInfoBranchesOff.insert("Invalid");
      else m_eventInfoTree->SetBranchAddress("Invalid", &invalid);
    }
  }

  if (treeMask & TRACKS)
    m_file.GetObject("Tracks", m_tracksTree);
  if (m_tracksTree) {
    m_tracksTree->SetBranchAddress("NTracks", &numTracks);
    if (!isTracksBranchOff("SlopeX")) {
      if (!m_tracksTree->GetBranch("SlopeX")) m_tracksBranchesOff.insert("SlopeX");
      else m_tracksTree->SetBranchAddress("SlopeX", trackSlopeX);
    }
    if (!isTracksBranchOff("SlopeY")) {
      if (!m_tracksTree->GetBranch("SlopeY")) m_tracksBranchesOff.insert("SlopeY");
      else m_tracksTree->SetBranchAddress("SlopeY", trackSlopeY);
    }
    if (!isTracksBranchOff("SlopeErrX")) {
      if (!m_tracksTree->GetBranch("SlopeErrX")) m_tracksBranchesOff.insert("SlopeErrX");
      else m_tracksTree->SetBranchAddress("SlopeErrX", trackSlopeErrX);
    }
    if (!isTracksBranchOff("SlopeErrY")) {
      if (!m_tracksTree->GetBranch("SlopeErrY")) m_tracksBranchesOff.insert("SlopeErrY");
      else m_tracksTree->SetBranchAddress("SlopeErrY", trackSlopeErrY);
    }
    if (!isTracksBranchOff("OriginX")) {
      if (!m_tracksTree->GetBranch("OriginX")) m_tracksBranchesOff.insert("OriginX");
      else m_tracksTree->SetBranchAddress("OriginX", trackOriginX);
    }
    if (!isTracksBranchOff("OriginY")) {
      if (!m_tracksTree->GetBranch("OriginY")) m_tracksBranchesOff.insert("OriginY");
      else m_tracksTree->SetBranchAddress("OriginY", trackOriginY);
    }
    if (!isTracksBranchOff("OriginErrX")) {
      if (!m_tracksTree->GetBranch("OriginErrX")) m_tracksBranchesOff.insert("OriginErrX");
      else m_tracksTree->SetBranchAddress("OriginErrX", trackOriginErrX);
    }
    if (!isTracksBranchOff("OriginErrY")) {
      if (!m_tracksTree->GetBranch("OriginErrY")) m_tracksBranchesOff.insert("OriginErrY");
      else m_tracksTree->SetBranchAddress("OriginErrY", trackOriginErrY);
    }
    if (!isTracksBranchOff("CovarianceX")) {
      if (!m_tracksTree->GetBranch("CovarianceX")) m_tracksBranchesOff.insert("CovarianceX");
      else m_tracksTree->SetBranchAddress("CovarianceX", trackCovarianceX);
    }
    if (!isTracksBranchOff("CovarianceY")) {
      if (!m_tracksTree->GetBranch("CovarianceY")) m_tracksBranchesOff.insert("CovarianceY");
      else m_tracksTree->SetBranchAddress("CovarianceY", trackCovarianceY);
    }
    if (!isTracksBranchOff("Chi2")) {
      if (!m_tracksTree->GetBranch("Chi2")) m_tracksBranchesOff.insert("Chi2");
      else m_tracksTree->SetBranchAddress("Chi2", trackChi2);
    }
  }

  // Check if tracks are given, clustesr are given, but clusters aren't
  // associated to tracks
  if (!m_tracksTree &&
      !m_clustersTrees.empty() &&
      isClustersBranchOff("InTrack"))
    throw std::runtime_error(
        "StorageI::StorageI: tracks are provided without cluster associations");

  // Test the validity of the file

  Long64_t nEventInfo = (m_eventInfoTree) ? m_eventInfoTree->GetEntries() : 0;
  Long64_t nTracks = (m_tracksTree) ? m_tracksTree->GetEntries() : 0;
  Long64_t nHits = 0;
  Long64_t nClusters = 0;
  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
    if (!m_hitsTrees.empty())
        nHits += m_hitsTrees[nplane]->GetEntries();
    if (!m_clustersTrees.empty())
        nClusters += m_clustersTrees[nplane]->GetEntriesFast();
  }

  if (nHits % m_numPlanes || nClusters % m_numPlanes)
    throw std::runtime_error(
        "StorageI::StorageI: number of events in different planes mismatch");

  nHits /= m_numPlanes;
  nClusters /= m_numPlanes;

  // Try to read the number of events from any active tree
  if (!m_numEvents && nEventInfo) m_numEvents = nEventInfo;
  else if (!m_numEvents && nTracks) m_numEvents = nTracks;
  else if (!m_numEvents && nHits) m_numEvents = nHits;
  else if (!m_numEvents && nClusters) m_numEvents = nClusters;

  // Confirm that all trees do have the same number of events
  if ((nEventInfo && m_numEvents != nEventInfo) ||
      (nTracks && m_numEvents != nTracks) ||
      (nHits && m_numEvents != nHits) ||
      (nClusters && m_numEvents != nClusters))
    throw std::runtime_error(
        "StoragI::StorageI: all trees don't have the same number of events");
}

Event& StorageI::readEvent(Long64_t n) {
  // NOTE: fill in reversed order: tracks first, hits last. This is so that
  // once a hit is produced, it can immediately recieve the address of its
  // parent cluster, likewise for clusters and track.

  // Don't try to access events that don't exit
  if (n >= m_numEvents)
    throw std::out_of_range(
        "StorageIO::readEvent: event out of bounds");

  // Try to read the event information
  if (m_eventInfoTree && m_eventInfoTree->GetEntry(n) <= 0)
    throw std::runtime_error(
        "StorageIO::error reading event tree");

  // Try to read the track information
  if (m_tracksTree && m_tracksTree->GetEntry(n) <= 0)
    throw std::runtime_error(
        "StorageIO::readEvent: error reading tracks tree");

  // NOTE: masks need to be re-applied here. The array values aren't zeroed
  // so they can't be read in

  // This will clear the previous event and cache its objects
  Event& event = newEvent();
  // Fill the event info fro what was read from the event info tree
  event.setTimeStamp(timeStamp);
  event.setFrameNumber(frameNumber);
  event.setTriggerOffset(triggerOffset);
  event.setTriggerInfo(triggerInfo);
  event.setInvalid(invalid);

  // Generate a list of track objects based on tracks from the tracks tree
  for (Int_t ntrack = 0; ntrack < numTracks; ntrack++) {
    // Ask the event to prepare a new track (comes from this objects' cache)
    Track& track = event.newTrack();
    track.setOrigin(trackOriginX[ntrack], trackOriginY[ntrack]);
    track.setOriginErr(trackOriginErrX[ntrack], trackOriginErrY[ntrack]);
    track.setSlope(trackSlopeX[ntrack], trackSlopeY[ntrack]);
    track.setSlopeErr(trackSlopeErrX[ntrack], trackSlopeErrY[ntrack]);
    track.setCovariance(trackCovarianceX[ntrack], trackCovarianceY[ntrack]);
    track.setChi2(trackChi2[ntrack]);
  }

  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
    // Try to read the hits tree for this plane
    if (!m_hitsTrees.empty() && m_hitsTrees[nplane]->GetEntry(n) <= 0)
      throw std::runtime_error(
          "StorageIO::readEvent: error reading hits tree");
    
    // Try to read the clusters tree for this plane
    if (!m_clustersTrees.empty() && m_clustersTrees[nplane]->GetEntry(n) <= 0)
      throw std::runtime_error(
          "StorageIO::readEvent: error reading clusters tree");

    // Generate the cluster objects
    for (Int_t ncluster = 0; ncluster < numClusters; ncluster++) {
      Cluster& cluster = event.newCluster(nplane);
      cluster.setPix(clusterPixX[ncluster], clusterPixY[ncluster]);
      cluster.setPixErr(clusterPixErrX[ncluster], clusterPixErrY[ncluster]);
      cluster.setPos(
          clusterPosX[ncluster],
          clusterPosY[ncluster],
          clusterPosZ[ncluster]);
      cluster.setPosErr(
          clusterPosErrX[ncluster],
          clusterPosErrY[ncluster],
          clusterPosErrZ[ncluster]);
      cluster.setTiming(clusterTiming[ncluster]);
      cluster.setValue(clusterValue[ncluster]);

      // If this cluster is in a track, mark this (and the tracks tree is active)
      if (m_tracksTree && clusterInTrack[ncluster] > 0) {
        Track& track = event.getTrack(clusterInTrack[ncluster]-1);
        track.addCluster(cluster);  // Bidirectional linking
      }
    }

    // Generate a list of all hit objects
    for (Int_t nhit = 0; nhit < numHits; nhit++) {
      const bool isMasked = 
          !m_noiseMasks.empty() &&
          m_noiseMasks[nplane].at(hitPixX[nhit], hitPixY[nhit]);

      // Don't make a hit object for masked hits if they are to be removed.
      // This will also prevent it being written out.
      if (isMasked && m_maskMode == REMOVE) {
        // If the hit was clustered, the cluster will be broken (it will try to
        // use a non-existent hit)
        if (hitInCluster[nhit] > 0) throw std::runtime_error(
              "StorageIO::readEvent: masking tried to remove a clustered hit");
        continue;
      }

      Hit& hit = event.newHit(nplane);
      hit.setPix(hitPixX[nhit], hitPixY[nhit]);
      hit.setPos(hitPosX[nhit], hitPosY[nhit], hitPosZ[nhit]);
      hit.setValue(hitValue[nhit]);
      hit.setTiming(hitTiming[nhit]);
      hit.setMasked(isMasked);  // Possible the hit is masked but not to be removed

      // If this hit is in a cluster, mark this (and the clusters tree is active)
      if (!m_clustersTrees.empty() && hitInCluster[nhit] > 0) {
        Cluster& cluster = event.getCluster(hitInCluster[nhit]-1);
        cluster.addHit(hit);  // Bidirectional linking
      }
    }
  }  // Loop over planes

  return event;
}

}
