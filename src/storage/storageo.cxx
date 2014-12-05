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
#include "storage/storageo.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

namespace Storage {

StorageO::StorageO(
    const std::string& filePath,
    size_t numPlanes,
    int treeMask) :
    StorageIO(filePath, OUTPUT, numPlanes, treeMask) {

  // Make hit and clusters trees for all the planes
  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
    // Make a directory for this plane
    std::stringstream ss;
    ss << "Plane" << nplane;  // Directories are named PlaneX
    TDirectory* dir = m_file->mkdir(ss.str().c_str());
    dir->cd();

    if (!(treeMask & HITS)) {
      TTree* hits = new TTree("Hits", "Hits");
      m_hitsTrees.push_back(hits);
      hits->Branch("NHits", &numHits, "NHits/I");
      hits->Branch("PixX", hitPixX, "HitPixX[NHits]/I");
      hits->Branch("PixY", hitPixY, "HitPixY[NHits]/I");
      hits->Branch("Value", hitValue, "HitValue[NHits]/D");
      hits->Branch("Timing", hitTiming, "HitTiming[NHits]/D");
      hits->Branch("InCluster", hitInCluster, "HitInCluster[NHits]/I");
      hits->Branch("PosX", hitPosX, "HitPosX[NHits]/D");
      hits->Branch("PosY", hitPosY, "HitPosY[NHits]/D");
      hits->Branch("PosZ", hitPosZ, "HitPosZ[NHits]/D");
    }

    if (!(treeMask & CLUSTERS)) {
      TTree* clusters = new TTree("Clusters", "Clusters");
      m_clustersTrees.push_back(clusters);
      clusters->Branch("NClusters", &numClusters, "NClusters/I");
      clusters->Branch("PixX", clusterPixX, "ClusterPixX[NClusters]/D");
      clusters->Branch("PixY", clusterPixY, "ClusterPixY[NClusters]/D");
      clusters->Branch("PixErrX", clusterPixErrX, "ClusterPixErrX[NClusters]/D");
      clusters->Branch("PixErrY", clusterPixErrY, "ClusterPixErrY[NClusters]/D");
      clusters->Branch("InTrack", clusterInTrack, "ClusterInTrack[NClusters]/I");
      clusters->Branch("PosX", clusterPosX, "ClusterPosX[NClusters]/D");
      clusters->Branch("PosY", clusterPosY, "ClusterPosY[NClusters]/D");
      clusters->Branch("PosZ", clusterPosZ, "ClusterPosZ[NClusters]/D");
      clusters->Branch("PosErrX", clusterPosErrX, "ClusterPosErrX[NClusters]/D");
      clusters->Branch("PosErrY", clusterPosErrY, "ClusterPosErrY[NClusters]/D");
      clusters->Branch("PosErrZ", clusterPosErrZ, "ClusterPosErrZ[NClusters]/D");
    }
  }  // Loop over planes

  if (!(treeMask & EVENTINFO)) {
    m_eventInfoTree = new TTree("Event", "Event information");
    m_eventInfoTree->Branch("TimeStamp", &timeStamp, "TimeStamp/l");
    m_eventInfoTree->Branch("FrameNumber", &frameNumber, "FrameNumber/l");
    m_eventInfoTree->Branch("TriggerOffset", &triggerOffset, "TriggerOffset/I");
    m_eventInfoTree->Branch("TriggerInfo", &triggerInfo, "TriggerInfo/I");
    m_eventInfoTree->Branch("Invalid", &invalid, "Invalid/O");
  }

  if (!(treeMask & TRACKS)) {
    m_tracksTree = new TTree("Tracks", "Track parameters");
    m_tracksTree->Branch("NTracks", &numTracks, "NTracks/I");
    m_tracksTree->Branch("SlopeX", trackSlopeX, "TrackSlopeX[NTracks]/D");
    m_tracksTree->Branch("SlopeY", trackSlopeY, "TrackSlopeY[NTracks]/D");
    m_tracksTree->Branch("SlopeErrX", trackSlopeErrX, "TrackSlopeErrX[NTracks]/D");
    m_tracksTree->Branch("SlopeErrY", trackSlopeErrY, "TrackSlopeErrY[NTracks]/D");
    m_tracksTree->Branch("OriginX", trackOriginX, "TrackOriginX[NTracks]/D");
    m_tracksTree->Branch("OriginY", trackOriginY, "TrackOriginY[NTracks]/D");
    m_tracksTree->Branch("OriginErrX", trackOriginErrX, "TrackOriginErrX[NTracks]/D");
    m_tracksTree->Branch("OriginErrY", trackOriginErrY, "TrackOriginErrY[NTracks]/D");
    m_tracksTree->Branch("CovarianceX", trackCovarianceX, "TrackCovarianceX[NTracks]/D");
    m_tracksTree->Branch("CovarianceY", trackCovarianceY, "TrackCovarianceY[NTracks]/D");
    m_tracksTree->Branch("Chi2", trackChi2, "TrackChi2[NTracks]/D");
  }
}

void StorageO::writeEvent(Event& event) {
  clearVariables();  // clear any lingering values

  m_file->cd();  // Ensure writing to the output file

  // Set the event information in local memory to be read into the file
  timeStamp = event.getTimeStamp();
  frameNumber = event.getFrameNumber();
  triggerOffset = event.getTriggerOffset();
  triggerInfo = event.getTriggerInfo();
  invalid = event.getInvalid();

  // Make sure there is enough space allocated to store all the tracks
  numTracks = event.getNumTracks();
  if (numTracks > MAX_TRACKS) 
    throw std::runtime_error(
        "StorageIO: event exceeds MAX_TRACKS");

  // Set the object track values into the arrays for writing to the root file
  for (int ntrack = 0; ntrack < numTracks; ntrack++) {
    Track& track = event.getTrack(ntrack);
    trackOriginX[ntrack] = track.getOriginX();
    trackOriginY[ntrack] = track.getOriginY();
    trackOriginErrX[ntrack] = track.getOriginErrX();
    trackOriginErrY[ntrack] = track.getOriginErrY();
    trackSlopeX[ntrack] = track.getSlopeX();
    trackSlopeY[ntrack] = track.getSlopeY();
    trackSlopeErrX[ntrack] = track.getSlopeErrX();
    trackSlopeErrY[ntrack] = track.getSlopeErrY();
    trackCovarianceX[ntrack] = track.getCovarianceX();
    trackCovarianceY[ntrack] = track.getCovarianceY();
    trackChi2[ntrack] = track.getChi2();
  }

  // Fill the hits and clusters trees one plane at a time
  for (unsigned int nplane = 0; nplane < m_numPlanes; nplane++) {
    Plane& plane = event.getPlane(nplane);

    // There might not be any hit trees or cluster trees, so check first before
    // comparing number of planes
    if ((!m_hitsTrees.empty() && nplane >= m_hitsTrees.size()) ||
        (!m_clustersTrees.empty() && nplane >= m_clustersTrees.size()))
      throw std::runtime_error(
          "StorageO::writeEvent: event has too many planes for the storage");

    numClusters = plane.getNumClusters();
    if (numClusters > MAX_CLUSTERS)
      throw std::runtime_error(
          "StorageO::writeEvent: event exceeds MAX_CLUSTERS");

    // Set the object cluster values into the arrays for writig into the root file
    for (int ncluster = 0; ncluster < numClusters; ncluster++) {
      Cluster& cluster = plane.getCluster(ncluster);
      clusterPixX[ncluster] = cluster.getPixX();
      clusterPixY[ncluster] = cluster.getPixY();
      clusterPixErrX[ncluster] = cluster.getPixErrX();
      clusterPixErrY[ncluster] = cluster.getPixErrY();
      clusterPosX[ncluster] = cluster.getPosX();
      clusterPosY[ncluster] = cluster.getPosY();
      clusterPosZ[ncluster] = cluster.getPosZ();
      clusterPosErrX[ncluster] = cluster.getPosErrX();
      clusterPosErrY[ncluster] = cluster.getPosErrY();
      clusterPosErrZ[ncluster] = cluster.getPosErrZ();
      clusterInTrack[ncluster] = cluster.fetchTrack() ? cluster.fetchTrack()->getIndex() : -1;
    }

    numHits = plane.getNumHits();
    if (numHits > MAX_HITS)
      throw std::runtime_error(
          "StorageO::writeEvent: event exceeds MAX_HITS");

    for (int nhit = 0; nhit < numHits; nhit++) {
      Hit& hit = plane.getHit(nhit);
      hitPixX[nhit] = hit.getPixX();
      hitPixY[nhit] = hit.getPixY();
      hitPosX[nhit] = hit.getPosX();
      hitPosY[nhit] = hit.getPosY();
      hitPosZ[nhit] = hit.getPosZ();
      hitValue[nhit] = hit.getValue();
      hitTiming[nhit] = hit.getTiming();
      hitInCluster[nhit] = hit.fetchCluster() ? hit.fetchCluster()->getIndex() : -1;
    }

    if (!m_hitsTrees.empty())
      m_hitsTrees[nplane]->Fill();
    if (!m_clustersTrees.empty())
      m_clustersTrees[nplane]->Fill();
  }

  // Write the track and event info here so that if any errors occured they
  // won't be desynchronized
  if (m_tracksTree) m_tracksTree->Fill();
  if (m_eventInfoTree) m_eventInfoTree->Fill();

  m_numEvents += 1;
}

}
