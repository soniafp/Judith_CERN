#include <iostream>
#include <cassert>
#include <vector>
#include <stdexcept>

#include "storage/hit.h"
#include "storage/cluster.h"

namespace Storage {

Cluster::Cluster() :
    m_track(0),
    m_matchedTrack(0),
    m_matchDistance(0),
    m_pixX(0),
    m_pixY(0),
    m_pixErrX(0),
    m_pixErrY(0),
    m_posX(0),
    m_posY(0),
    m_posZ(0),
    m_posErrX(0),
    m_posErrY(0),
    m_posErrZ(0),
    m_timing(0),
    m_value(0),
    m_index(-1),
    m_plane(0) {}

void Cluster::clear() {
  m_hits.clear();
  m_track = 0;
  m_matchedTrack = 0;
  m_matchDistance = 0;
  m_pixX = 0;
  m_pixY = 0;
  m_pixErrX = 0;
  m_pixErrY = 0;
  m_posX = 0;
  m_posY = 0;
  m_posZ = 0;
  m_posErrX = 0;
  m_posErrY = 0;
  m_posErrZ = 0;
  m_timing = 0;
  m_value = 0;
  m_index = -1;
  m_plane = 0;
}

void Cluster::print() {
  std::cout <<
      "\nCLUSTER:\n"
      "  Address:  " << this << "\n"
      "  Pix:      (" << m_pixX << " , " << m_pixY << ")\n"
      "  Pix err:  (" << m_pixErrX << " , " << m_pixErrY << ")\n"
      "  Pos:      (" << m_posX << " , " << m_posY << " , " << m_posZ << ")\n"
      "  Pos err:  (" << m_posErrX << " , " << m_posErrY << " , " << m_posErrZ << ")\n"
      "  Num hits: " << getNumHits() << "\n"
      "  Plane:    "  << fetchPlane() << std::endl;
}

void Cluster::setTrack(Track& track) {
  if (m_track)
    throw std::runtime_error(
        "Cluster::setTrack: cluster already in a track");
  m_track = &track;
}

void Cluster::addHit(Hit& hit) {
  hit.setCluster(*this);
  m_hits.push_back(&hit);
}

Hit& Cluster::getHit(size_t n) const {
  if (n >= getNumHits())
    throw std::out_of_range(
        "Cluster::getHit: requested hit out of range");
  return *m_hits[n];
}

}

