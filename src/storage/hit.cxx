#include <iostream>
#include <cassert>
#include <stdexcept>

#include "storage/hit.h"

namespace Storage {

Hit::Hit() :
    m_cluster(0),
    m_pixX(0),
    m_pixY(0),
    m_posX(0),
    m_posY(0),
    m_posZ(0),
    m_value(0),
    m_timing(0),
    m_masked(false),
    m_plane(0) {}

void Hit::clear() {
  m_cluster = 0;
  m_pixX = 0;
  m_pixY = 0;
  m_posX = 0;
  m_posY = 0;
  m_posZ = 0;
  m_value = 0;
  m_timing = 0;
  m_masked = false;
  m_plane = 0;
}

void Hit::print() {
  std::cout << 
      "\nHIT:\n"
      "  Address: " << this << "\n"
      "  Pix:     (" << m_pixX << " , " << m_pixY << ")\n"
      "  Pos:     (" << m_posX << " , " << m_posY << " , " << m_posZ << ")\n"
      "  Value:   " << m_value << "\n"
      "  Timing:  " << m_timing << "\n"
      "  Cluster: " << fetchCluster() << "\n"
      "  Plane:   "  << fetchPlane() << std::endl;
}

void Hit::setCluster(Cluster& cluster) {
  if (m_cluster)
    throw std::runtime_error(
        "Hit::setCluster: hit already in a cluster");
  m_cluster = &cluster;
}

}
