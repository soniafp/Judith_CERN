#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

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
    const std::vector<bool>* planeMask) :
    StorageIO(filePath, INPUT, treeMask) {

  // Keep track of the number of planes read from the file
  size_t planeCount = 0;

  // Read all planes from the file
  while (true) {
    // Try to get this plane's directory
    std::stringstream ss;
    ss << "Plane" << planeCount;  // Directories are named PlaneX
    TDirectory* dir = 0;
    m_file->GetObject(ss.str().c_str(), dir);

    // When no more planes are found, stop
    if (!dir) break;

    planeCount += 1;

    if (planeMask && planeCount > planeMask->size())
      throw std::runtime_error(
          "StorageI::StorageI: plane mask is too small");

    // Skip loading this plane from file if masked
    if (planeMask && planeMask->at(planeCount-1))
      continue;

    // Load the hits tree if it is found and not masked
    TTree* hits = 0;
    m_file->GetObject((ss.str()+"/Hits").c_str(), hits);
    if (!(treeMask & HITS) && hits) {
      // Add this tree to the current plane
      m_hitsTrees.push_back(hits);
      // Associate its branches to local memory
      hits->SetBranchAddress("NHits", &numHits);
      hits->SetBranchAddress("PixX", hitPixX);
      hits->SetBranchAddress("PixY", hitPixY);
      hits->SetBranchAddress("Value", hitValue);
      hits->SetBranchAddress("Timing", hitTiming);
      hits->SetBranchAddress("InCluster", hitInCluster);
      hits->SetBranchAddress("PosX", hitPosX);
      hits->SetBranchAddress("PosY", hitPosY);
      hits->SetBranchAddress("PosZ", hitPosZ);
    }

    TTree* clusters = 0;
    m_file->GetObject((ss.str()+"/Clusters").c_str(), clusters);
    if (!(treeMask & CLUSTERS) && clusters) {
      m_clustersTrees.push_back(clusters);
      clusters->SetBranchAddress("NClusters", &numClusters);
      clusters->SetBranchAddress("PixX", clusterPixX);
      clusters->SetBranchAddress("PixY", clusterPixY);
      clusters->SetBranchAddress("PixErrX", clusterPixErrX);
      clusters->SetBranchAddress("PixErrY", clusterPixErrY);
      clusters->SetBranchAddress("InTrack", clusterInTrack);
      clusters->SetBranchAddress("PosX", clusterPosX);
      clusters->SetBranchAddress("PosY", clusterPosY);
      clusters->SetBranchAddress("PosZ", clusterPosZ);
      clusters->SetBranchAddress("PosErrX", clusterPosErrX);
      clusters->SetBranchAddress("PosErrY", clusterPosErrY);
      clusters->SetBranchAddress("PosErrZ", clusterPosErrZ);
    }
  }  // Loop over planes

  if (!m_hitsTrees.empty() && !m_clustersTrees.empty() &&
      m_hitsTrees.size() != m_clustersTrees.size())
    throw std::runtime_error(
        "StorageI::StorageI: different number of planes in hits and cluters");

  // Number of planes is whichever has planes, or 0 
  m_numPlanes = (m_hitsTrees.empty()) ?
      m_clustersTrees.size() : m_hitsTrees.size();

  m_file->GetObject("Event", m_eventInfoTree);
  if (!(treeMask & EVENTINFO) && m_eventInfoTree) {
    m_eventInfoTree->SetBranchAddress("TimeStamp", &timeStamp);
    m_eventInfoTree->SetBranchAddress("FrameNumber", &frameNumber);
    m_eventInfoTree->SetBranchAddress("TriggerOffset", &triggerOffset);
    m_eventInfoTree->SetBranchAddress("TriggerInfo", &triggerInfo);
    m_eventInfoTree->SetBranchAddress("Invalid", &invalid);
  }

  m_file->GetObject("Tracks", m_tracksTree);
  if (!(treeMask & TRACKS) && m_tracksTree) {
    m_tracksTree->SetBranchAddress("NTracks", &numTracks);
    m_tracksTree->SetBranchAddress("SlopeX", trackSlopeX);
    m_tracksTree->SetBranchAddress("SlopeY", trackSlopeY);
    m_tracksTree->SetBranchAddress("SlopeErrX", trackSlopeErrX);
    m_tracksTree->SetBranchAddress("SlopeErrY", trackSlopeErrY);
    m_tracksTree->SetBranchAddress("OriginX", trackOriginX);
    m_tracksTree->SetBranchAddress("OriginY", trackOriginY);
    m_tracksTree->SetBranchAddress("OriginErrX", trackOriginErrX);
    m_tracksTree->SetBranchAddress("OriginErrY", trackOriginErrY);
    m_tracksTree->SetBranchAddress("CovarianceX", trackCovarianceX);
    m_tracksTree->SetBranchAddress("CovarianceY", trackCovarianceY);
    m_tracksTree->SetBranchAddress("Chi2", trackChi2);
  }

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
  if (!m_numEvents && nTracks) m_numEvents = nTracks;
  if (!m_numEvents && nHits) m_numEvents = nHits;
  if (!m_numEvents && nClusters) m_numEvents = nClusters;

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

  // Clear previous event
  clearVariables();

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

  // This will clear the previous event and cache its objects
  Event& event = newEvent();
  // Fill the event info fro what was read from the event info tree
  event.setTimeStamp(timeStamp);
  event.setFrameNumber(frameNumber);
  event.setTriggerOffset(triggerOffset);
  event.setTriggerInfo(triggerInfo);
  event.setInvalid(invalid);

  // Generate a list of track objects based on tracks from the tracks tree
  for (int ntrack = 0; ntrack < numTracks; ntrack++) {
    // Ask the event to prepare a new track (comes from this objects' cache)
    Track& track = event.newTrack();
    // Fill it with values read from the tree
    track.setOrigin(trackOriginX[ntrack], trackOriginY[ntrack]);
    track.setOriginErr(trackOriginErrX[ntrack], trackOriginErrY[ntrack]);
    track.setSlope(trackSlopeX[ntrack], trackSlopeY[ntrack]);
    track.setSlopeErr(trackSlopeErrX[ntrack], trackSlopeErrY[ntrack]);
    track.setCovariance(trackCovarianceX[ntrack], trackCovarianceY[ntrack]);
    track.setChi2(trackChi2[ntrack]);
  }

  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
    // Try to read the hits tree for this plane
    if (m_hitsTrees.at(nplane) && m_hitsTrees.at(nplane)->GetEntry(n) <= 0)
      throw std::runtime_error(
          "StorageIO::readEvent: error reading hits tree");
    
    // Try to read the clusters tree for this plane
    if (m_clustersTrees.at(nplane) && m_clustersTrees.at(nplane)->GetEntry(n) <= 0)
      throw std::runtime_error(
          "StorageIO::readEvent: error reading clusters tree");

    // Generate the cluster objects
    for (int ncluster = 0; ncluster < numClusters; ncluster++) {
      Cluster& cluster = event.newCluster(nplane);
      cluster.setPix(clusterPixX[ncluster], clusterPixY[ncluster]);
      cluster.setPixErr(clusterPixErrX[ncluster], clusterPixErrY[ncluster]);
      cluster.setPos(clusterPosX[ncluster], clusterPosY[ncluster], clusterPosZ[ncluster]);
      cluster.setPosErr(clusterPosErrX[ncluster], clusterPosErrY[ncluster], clusterPosErrZ[ncluster]);

      // If this cluster is in a track, mark this (and the tracks tree is active)
      if (m_tracksTree && clusterInTrack[ncluster] >= 0) {
        Track& track = event.getTrack(clusterInTrack[ncluster]);
        track.addCluster(cluster);
        cluster.setTrack(track);
      }
    }

    // Generate a list of all hit objects
    for (int nhit = 0; nhit < numHits; nhit++) {
      const bool isMasked = 
          !m_noiseMasks.empty() &&
          m_noiseMasks[nplane].at(hitPixX[nhit], hitPixY[nhit]);

      if (isMasked && m_maskMode == REMOVE) {
        if (hitInCluster[nhit] >= 0)
          throw std::runtime_error(
              "StorageIO::readEvent: tried to remove a clustered hit");
        continue;
      }

      Hit& hit = event.newHit(nplane);
      hit.setPix(hitPixX[nhit], hitPixY[nhit]);
      hit.setPos(hitPosX[nhit], hitPosY[nhit], hitPosZ[nhit]);
      hit.setValue(hitValue[nhit]);
      hit.setTiming(hitTiming[nhit]);
      hit.setMasked(isMasked);

      // If this hit is in a cluster, mark this (and the clusters tree is active)
      if (!m_clustersTrees.empty() && hitInCluster[nhit] >= 0) {
        Cluster& cluster = event.getCluster(hitInCluster[nhit]);
        cluster.addHit(hit);
      }
    }
  }  // Loop over planes

  return event;
}

}
