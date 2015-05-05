#include <iostream>
#include <cassert>
#include <vector>
#include <stack>
#include <string>
#include <stdexcept>
#include <set>
#include <cstring>

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
    size_t numPlanes) :
    m_file(filePath.c_str(), (fileMode==INPUT) ? "READ" : "RECREATE"),
    m_fileMode(fileMode),
    m_numPlanes(numPlanes),
    m_maskMode(REMOVE),
    m_numEvents(0),
    m_event(0),
    m_tracksTree(0),
    m_eventInfoTree(0) {
  if (!m_file.IsOpen()) throw std::runtime_error(
        "StorageIO::StorageIO: file didn't initialize");

  // Zero all the buffer memory at once. C++ guarantees continuous memory for
  // members declared sequentially not split by an access specifier, so this
  // works.

  // Total size from numHits to the end of the last track array
  size_t size = (size_t)(&trackChi2) - (size_t)(&numHits);
  size += sizeof(Double_t)*MAX_TRACKS;
  std::memset(&numHits, 0, size);
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
  for (std::vector<TTree*>::iterator it = m_hitsTrees.begin();
      it != m_hitsTrees.end(); ++it)
    if (*it) delete (*it);
  for (std::vector<TTree*>::iterator it = m_clustersTrees.begin();
      it != m_clustersTrees.end(); ++it)
    if (*it) delete (*it);
  if (m_tracksTree) delete m_tracksTree;
  if (m_eventInfoTree) delete m_eventInfoTree;

  m_file.Close();
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

bool StorageIO::isHitsBranchOff(const std::string& name) const {
  return m_hitsBranchesOff.find(name) != m_hitsBranchesOff.end();
}

bool StorageIO::isClustersBranchOff(const std::string& name) const {
  return m_clustersBranchesOff.find(name) != m_clustersBranchesOff.end();
}

bool StorageIO::isTracksBranchOff(const std::string& name) const {
  return m_tracksBranchesOff.find(name) != m_tracksBranchesOff.end();
}

bool StorageIO::isEventInfoBranchOff(const std::string& name) const {
  return m_eventInfoBranchesOff.find(name) != m_eventInfoBranchesOff.end();
}

}
