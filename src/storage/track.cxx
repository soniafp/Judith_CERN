#include <iostream>
#include <cassert>

#include "storage/cluster.h"
#include "storage/track.h"

namespace Storage {

Track::Track() :
    m_originX(0),
    m_originY(0),
    m_originErrX(0),
    m_originErrY(0),
    m_slopeX(0),
    m_slopeY(0),
    m_slopeErrX(0),
    m_slopeErrY(0),
    m_covarianceX(0),
    m_covarianceY(0),
    m_chi2(0),
    m_index(-1) {}

void Track::clear() {
  m_clusters.clear();
  m_matchedClusters.clear();
  m_originX = 0;
  m_originY = 0;
  m_originErrX = 0;
  m_originErrY = 0;
  m_slopeX = 0;
  m_slopeY = 0;
  m_slopeErrX = 0;
  m_slopeErrY = 0;
  m_covarianceX = 0;
  m_covarianceY = 0;
  m_chi2 = 0;
  m_index = -1;
}

void Track::addCluster(Cluster& cluster) {
  cluster.setTrack(*this);
  m_clusters.push_back(&cluster);
}

void Track::setClusters(const std::vector<Cluster*>& clusters) {
  m_clusters = clusters;
  for (std::vector<Cluster*>::iterator it = m_clusters.begin();
      it != m_clusters.end(); ++it)
    (*it)->setTrack(*this);
}

void Track::addMatchedCluster(Cluster& cluster) {
  m_matchedClusters.push_back(&cluster);
}

void Track::setMatchedClusters(const std::vector<Cluster*>& clusters) {
  m_matchedClusters = clusters;
}

}
