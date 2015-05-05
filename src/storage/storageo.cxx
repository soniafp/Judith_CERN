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
#include "storage/storageo.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

namespace Storage {

StorageO::StorageO(
    const std::string& filePath,
    size_t numPlanes,
    int treeMask,
    const std::set<std::string>* hitsBranchesOff,
    const std::set<std::string>* clustersBranchesOff,
    const std::set<std::string>* tracksBranchesOff,
    const std::set<std::string>* eventInfoBranchesOff) :
    StorageIO(filePath, OUTPUT, numPlanes) {

  // Copy any/all given branch masks
  if (hitsBranchesOff) m_hitsBranchesOff = *hitsBranchesOff;
  if (clustersBranchesOff) m_clustersBranchesOff = *clustersBranchesOff;
  if (tracksBranchesOff) m_tracksBranchesOff = *tracksBranchesOff;
  if (eventInfoBranchesOff) m_eventInfoBranchesOff = *eventInfoBranchesOff;

  // Avoid always checking !
  treeMask = ~treeMask;

  // Make hit and clusters trees for all the planes
  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
    // Make a directory for this plane
    std::stringstream ss;
    ss << "Plane" << nplane;  // Directories are named PlaneX

    // Make the hits and clusters trees in the corresponding plane directory
    TDirectory* dir = m_file.mkdir(ss.str().c_str());
    dir->cd();

    // Check if the hits tree is masked alltogether
    if (treeMask & HITS) {
      // Make a tree to store hits for this plane
      TTree* hitsTreePl = new TTree("Hits", "Hits");
      // Keep a pointer to the tree object
      m_hitsTrees.push_back(hitsTreePl);
      // Add a branch to track the number of hits per event
      hitsTreePl->Branch("NHits", &numHits, "NHits/I");
      // Check if the `PixX` branch has been turned off, and make the branch otherwise
      if (!isHitsBranchOff("PixX"))
        hitsTreePl->Branch("PixX", hitPixX, "HitPixX[NHits]/I");
      if (!isHitsBranchOff("PixY"))
        hitsTreePl->Branch("PixY", hitPixY, "HitPixY[NHits]/I");
      if (!isHitsBranchOff("PosX"))
        hitsTreePl->Branch("PosX", hitPosX, "HitPosX[NHits]/D");
      if (!isHitsBranchOff("PosY"))
        hitsTreePl->Branch("PosY", hitPosY, "HitPosY[NHits]/D");
      if (!isHitsBranchOff("PosZ"))
        hitsTreePl->Branch("PosZ", hitPosZ, "HitPosZ[NHits]/D");
      if (!isHitsBranchOff("Value"))
        hitsTreePl->Branch("Value", hitValue, "HitValue[NHits]/I");
      if (!isHitsBranchOff("Timing"))
        hitsTreePl->Branch("Timing", hitTiming, "HitTiming[NHits]/I");
      if (treeMask & CLUSTERS)
        hitsTreePl->Branch("InCluster", hitInCluster, "HitInCluster[NHits]/I");
    }

    if (treeMask & CLUSTERS) {
      TTree* clustersTreePl = new TTree("Clusters", "Clusters");
      m_clustersTrees.push_back(clustersTreePl);
      clustersTreePl->Branch("NClusters", &numClusters, "NClusters/I");
      if (!isClustersBranchOff("PixX"))
        clustersTreePl->Branch("PixX", clusterPixX, "ClusterPixX[NClusters]/D");
      if (!isClustersBranchOff("PixY"))
        clustersTreePl->Branch("PixY", clusterPixY, "ClusterPixY[NClusters]/D");
      if (!isClustersBranchOff("PixErrX"))
        clustersTreePl->Branch("PixErrX", clusterPixErrX, "ClusterPixErrX[NClusters]/D");
      if (!isClustersBranchOff("PixErrY"))
        clustersTreePl->Branch("PixErrY", clusterPixErrY, "ClusterPixErrY[NClusters]/D");
      if (!isClustersBranchOff("PosX"))
        clustersTreePl->Branch("PosX", clusterPosX, "ClusterPosX[NClusters]/D");
      if (!isClustersBranchOff("PosY"))
        clustersTreePl->Branch("PosY", clusterPosY, "ClusterPosY[NClusters]/D");
      if (!isClustersBranchOff("PosZ"))
        clustersTreePl->Branch("PosZ", clusterPosZ, "ClusterPosZ[NClusters]/D");
      if (!isClustersBranchOff("PosErrX"))
        clustersTreePl->Branch("PosErrX", clusterPosErrX, "ClusterPosErrX[NClusters]/D");
      if (!isClustersBranchOff("PosErrY"))
        clustersTreePl->Branch("PosErrY", clusterPosErrY, "ClusterPosErrY[NClusters]/D");
      if (!isClustersBranchOff("PosErrZ"))
        clustersTreePl->Branch("PosErrZ", clusterPosErrZ, "ClusterPosErrZ[NClusters]/D");
      if (!isClustersBranchOff("Value"))
        clustersTreePl->Branch("Value", clusterValue, "ClusterValue[NClusters]/D");
      if (!isClustersBranchOff("Timing"))
        clustersTreePl->Branch("Timing", clusterTiming, "ClusterTiming[NClusters]/D");
      if (treeMask & TRACKS)
        clustersTreePl->Branch("InTrack", clusterInTrack, "ClusterInTrack[NClusters]/I");
    }
  }  // Loop over planes

  // Make the event and track trees in the root directory
  m_file.cd();

  if (treeMask & EVENTINFO) {
    m_eventInfoTree = new TTree("Event", "Event information");
    if (!isEventInfoBranchOff("TimeStamp"))
      m_eventInfoTree->Branch("TimeStamp", &timeStamp, "TimeStamp/l");
    if (!isEventInfoBranchOff("FrameNumber"))
      m_eventInfoTree->Branch("FrameNumber", &frameNumber, "FrameNumber/l");
    if (!isEventInfoBranchOff("TriggerOffset"))
      m_eventInfoTree->Branch("TriggerOffset", &triggerOffset, "TriggerOffset/I");
    if (!isEventInfoBranchOff("TriggerInfo"))
      m_eventInfoTree->Branch("TriggerInfo", &triggerInfo, "TriggerInfo/I");
    if (!isEventInfoBranchOff("Invalid"))
      m_eventInfoTree->Branch("Invalid", &invalid, "Invalid/O");
  }

  if (treeMask & TRACKS) {
    m_tracksTree = new TTree("Tracks", "Track parameters");
    m_tracksTree->Branch("NTracks", &numTracks, "NTracks/I");
    if (!isTracksBranchOff("SlopeX"))
      m_tracksTree->Branch("SlopeX", trackSlopeX, "TrackSlopeX[NTracks]/D");
    if (!isTracksBranchOff("SlopeY"))
      m_tracksTree->Branch("SlopeY", trackSlopeY, "TrackSlopeY[NTracks]/D");
    if (!isTracksBranchOff("SlopeErrX"))
      m_tracksTree->Branch("SlopeErrX", trackSlopeErrX, "TrackSlopeErrX[NTracks]/D");
    if (!isTracksBranchOff("SlopeErrY"))
      m_tracksTree->Branch("SlopeErrY", trackSlopeErrY, "TrackSlopeErrY[NTracks]/D");
    if (!isTracksBranchOff("OriginX"))
      m_tracksTree->Branch("OriginX", trackOriginX, "TrackOriginX[NTracks]/D");
    if (!isTracksBranchOff("OriginY"))
      m_tracksTree->Branch("OriginY", trackOriginY, "TrackOriginY[NTracks]/D");
    if (!isTracksBranchOff("OriginErrX"))
      m_tracksTree->Branch("OriginErrX", trackOriginErrX, "TrackOriginErrX[NTracks]/D");
    if (!isTracksBranchOff("OriginErrY"))
      m_tracksTree->Branch("OriginErrY", trackOriginErrY, "TrackOriginErrY[NTracks]/D");
    if (!isTracksBranchOff("CovarianceX"))
      m_tracksTree->Branch("CovarianceX", trackCovarianceX, "TrackCovarianceX[NTracks]/D");
    if (!isTracksBranchOff("CovarianceY"))
      m_tracksTree->Branch("CovarianceY", trackCovarianceY, "TrackCovarianceY[NTracks]/D");
    if (!isTracksBranchOff("Chi2"))
      m_tracksTree->Branch("Chi2", trackChi2, "TrackChi2[NTracks]/D");
  }
}

StorageO::~StorageO() {
  m_file.Write();
}

void StorageO::writeEvent(Event& event) {
  m_file.cd();  // Ensure writing to the output file

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
  for (Int_t ntrack = 0; ntrack < numTracks; ntrack++) {
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
  for (size_t nplane = 0; nplane < m_numPlanes; nplane++) {
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
    for (Int_t ncluster = 0; ncluster < numClusters; ncluster++) {
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
      clusterTiming[ncluster] = cluster.getTiming();
      clusterValue[ncluster] = cluster.getValue();
      clusterInTrack[ncluster] = cluster.fetchTrack() ? cluster.fetchTrack()->getIndex() : 0;
    }

    numHits = plane.getNumHits();
    if (numHits > MAX_HITS)
      throw std::runtime_error(
          "StorageO::writeEvent: event exceeds MAX_HITS");

    for (Int_t nhit = 0; nhit < numHits; nhit++) {
      Hit& hit = plane.getHit(nhit);
      hitPixX[nhit] = hit.getPixX();
      hitPixY[nhit] = hit.getPixY();
      hitPosX[nhit] = hit.getPosX();
      hitPosY[nhit] = hit.getPosY();
      hitPosZ[nhit] = hit.getPosZ();
      hitValue[nhit] = hit.getValue();
      hitTiming[nhit] = hit.getTiming();
      hitInCluster[nhit] = hit.fetchCluster() ? hit.fetchCluster()->getIndex(): 0;
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
