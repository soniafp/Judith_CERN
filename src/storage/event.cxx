#include <iostream>
#include <cassert>
#include <vector>
#include <stdexcept>

#include <Rtypes.h>

#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/track.h"
#include "storage/plane.h"
#include "storage/storageio.h"
#include "storage/event.h"

namespace Storage {

Event::Event(StorageIO& storage) :
    m_storage(&storage),
    m_planes(m_storage->getNumPlanes(), 0),
    m_timeStamp(0),
    m_frameNumber(0),
    m_triggerOffset(0),
    m_triggerInfo(0),
    m_invalid(false) {
  if (m_storage->m_event)
    throw std::runtime_error("Event::Event: StorageIO already owns an event");
  // Allocate the planes used to associate hits and clusters
  for (size_t i = 0; i < m_planes.size(); i++)
    m_planes[i] = new Plane(i);
}

Event::Event(size_t numPlanes) :
    m_storage(0),  // Not owned by StorageIO
    m_planes(numPlanes, 0),
    m_timeStamp(0),
    m_frameNumber(0),
    m_triggerOffset(0),
    m_triggerInfo(0),
    m_invalid(false) {
  // Allocate the planes used to associate hits and clusters
  for (size_t i = 0; i < m_planes.size(); i++)
    m_planes[i] = new Plane(i);
}

Event::~Event() {
  for (std::vector<Hit*>::iterator it = m_hits.begin();
      it != m_hits.end(); ++it)
    delete (*it);
  for (std::vector<Cluster*>::iterator it = m_clusters.begin();
      it != m_clusters.end(); ++it)
    delete (*it);
  for (std::vector<Plane*>::iterator it = m_planes.begin();
      it != m_planes.end(); ++it)
    delete (*it);
  for (std::vector<Track*>::iterator it = m_tracks.begin();
      it != m_tracks.end(); ++it)
    delete (*it);
}

void Event::clear() {
  // WARNING: clearing will reqlinquish ownership of objects within
  m_hits.clear();
  m_clusters.clear();
  m_tracks.clear();

  m_timeStamp = 0;
  m_frameNumber = 0;
  m_triggerOffset = 0;
  m_triggerInfo = 0;
  m_invalid = false;
}

void Event::print() {
  std::cout
      << "\nEVENT:\n"
      << "  Time stamp: " << getTimeStamp() << "\n"
      << "  Frame number: " << getFrameNumber() << "\n"
      << "  Trigger offset: " << getTriggerOffset() << "\n"
      << "  Invalid: " << getInvalid() << "\n"
      << "  Num planes: " << getNumPlanes() << "\n"
      << "  Num hits: " << getNumHits() << "\n"
      << "  Num clusters: " << getNumClusters() << std::endl;

  for (unsigned int nplane = 0; nplane < getNumPlanes(); nplane++)
    getPlane(nplane).print();
}

Hit& Event::newHit(size_t nplane) {
  Hit* hit = 0;
  // If the event is managed by storage, use its cache for the hit
  if (m_storage) {
    hit = &m_storage->newHit();
    hit->clear();  // Clear prior values
  } else {
    hit = new Hit();
  }
  // This event now owns the hit's memory, unless it is cleared
  m_hits.push_back(hit);
  // Do the two way plane association
  m_planes.at(nplane)->m_hits.push_back(hit);
  hit->m_plane = m_planes.at(nplane);
  // Return a reference to make ownership clear
  return *hit;
}

Cluster& Event::newCluster(size_t nplane) {
  Cluster* cluster = 0;
  if (m_storage) {
    cluster = &m_storage->newCluster();
  } else {
    cluster = new Cluster();
  }
  cluster->m_index = getNumClusters();
  m_clusters.push_back(cluster);
  m_planes.at(nplane)->m_clusters.push_back(cluster);
  cluster->m_plane = m_planes.at(nplane);
  return *cluster;
}

Track& Event::newTrack() {
  Track* track = 0;
  if (m_storage) {
    track = &m_storage->newTrack();
  } else {
    track = new Track();
  }
  track->m_index = getNumTracks();
  m_tracks.push_back(track);
  return *track;
}

}
