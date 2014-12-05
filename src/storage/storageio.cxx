#include <iostream>
#include <cassert>
#include <vector>
#include <stack>
#include <string>
#include <stdexcept>

#include <TTree.h>

#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/track.h"
#include "storage/event.h"
#include "storage/storageio.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

namespace Storage {

StorageIO::StorageIO(
    const std::string& filePath,
    FileMode fileMode,
    size_t numPlanes,
    int treeMask) :
    m_file(0),
    m_fileMode(fileMode),
    m_numPlanes(numPlanes),
    m_maskMode(REMOVE),
    m_numEvents(0),
    m_event(0),
    m_tracksTree(0),
    m_eventInfoTree(0) {
  if (fileMode == INPUT)
    m_file = new TFile(filePath.c_str(), "READ");
  else if (fileMode == OUTPUT)
    m_file = new TFile(filePath.c_str(), "RECREATE");

  if (!m_file)
    throw std::runtime_error(
        "StorageIO::StorageIO: file didn't initialize");

  clearVariables();
}

StorageIO::~StorageIO() {
  // Delete the chached event
  if (m_event) delete m_event;

  // Clear cache
  while (!m_cacheHits.empty()) {
    delete m_cacheHits.top();
    m_cacheHits.pop();
  }
  while (!m_cacheClusters.empty()) {
    delete m_cacheClusters.top();
    m_cacheClusters.pop();
  }
  while (!m_cacheTracks.empty()) {
    delete m_cacheTracks.top();
    m_cacheTracks.pop();
  }

  // Clear trees
  for (std::vector<TTree*>::iterator it = m_clustersTrees.begin();
      it != m_clustersTrees.end(); ++it)
    if (*it) delete (*it);
  for (std::vector<TTree*>::iterator it = m_clustersTrees.begin();
      it != m_clustersTrees.end(); ++it)
    if (*it) delete (*it);
  if (m_tracksTree) delete m_tracksTree;
  if (m_eventInfoTree) delete m_eventInfoTree;

  // Write, close and delete the file as approrpriate
  if (m_file) {
    if (m_fileMode == OUTPUT)
      m_file->Write();
    m_file->Close();
    delete m_file;
  }
}

void StorageIO::clearVariables() {
  timeStamp = 0;
  frameNumber = 0;
  triggerOffset = 0;
  invalid = false;
  numHits = 0;
  numClusters = 0;
  numTracks = 0;
}

Event& StorageIO::newEvent() {
  if (!m_event) {
    // First call will generate the event object used by the storage
    m_event = new Event(*this);
  } else {
    // The current `m_event` will be overwritten. Take onwership of the event's
    // objects and cache them
    for (std::vector<Hit*>::iterator it = m_event->m_hits.begin();
        it != m_event->m_hits.end(); ++it)
      m_cacheHits.push(*it);
    for (std::vector<Cluster*>::iterator it = m_event->m_clusters.begin();
        it != m_event->m_clusters.end(); ++it)
      m_cacheClusters.push(*it);
    for (std::vector<Track*>::iterator it = m_event->m_tracks.begin();
        it != m_event->m_tracks.end(); ++it)
      m_cacheTracks.push(*it);
    // Clear the event's state and object associations
    m_event->clear();
  }

  return *m_event;
}

Track& StorageIO::newTrack() {
  Track* track = 0;
  // Look for track in cache
  if (m_cacheTracks.empty()) {
    // Cahce is empty, make new track
    track = new Track();
  } else {
    // Cache has tracks, get last
    track = m_cacheTracks.top();
    // Remove from cache, calling `Event` now owns it
    m_cacheTracks.pop();
    // Clear prior state and associations
    track->clear();
  }
  return *track;
}

Cluster& StorageIO::newCluster() {
  Cluster* cluster = 0;
  if (m_cacheClusters.empty()) {
    cluster = new Cluster();
  } else {
    cluster = m_cacheClusters.top();
    m_cacheClusters.pop();
    cluster->clear();
  }
  return *cluster;
}

Hit& StorageIO::newHit() {
  Hit* hit = 0;
  if (m_cacheHits.empty()) {
    hit = new Hit();
  } else {
    hit = m_cacheHits.top();
    m_cacheHits.pop();
    hit->clear();
  }
  return *hit;
}

// Disable the branch by the given name in the given tree, throw an exception
// if the branch doesn't exist
void disableBranch(TTree& tree, const std::string& name) {
  UInt_t found = 0;
  tree.SetBranchStatus(name.c_str(), 0, &found);
  if (!found) {
    std::cerr << "ERROR: branch not found: " << name 
        << " in tree " << tree.GetName() << std::endl;
    throw std::runtime_error(
        "StorageIO: disableBranch: no such branch");
  }
}

void StorageIO::disableHitsBranch(const std::string& name) {
  for (std::vector<TTree*>::iterator it = m_hitsTrees.begin();
      it != m_hitsTrees.end(); ++it)
    disableBranch(**it, name);
}

void StorageIO::disableClustersBranch(const std::string& name) {
  for (std::vector<TTree*>::iterator it = m_clustersTrees.begin();
      it != m_clustersTrees.end(); ++it)
    disableBranch(**it, name);
}

void StorageIO::disableTracksBranch(const std::string& name) {
  if (m_tracksTree) disableBranch(*m_tracksTree, name);
}

void StorageIO::disableEventInfoBranch(const std::string& name) {
  if (m_eventInfoTree) disableBranch(*m_eventInfoTree, name);
}

}
