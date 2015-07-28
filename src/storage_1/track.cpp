#include "track.h"

#include <cassert>

#include "cluster.h"

namespace Storage {

// NOTE: this doesn't tell the cluster about the track (due to building trial tracks)
void Track::addCluster(Cluster* cluster)
{
  _clusters.push_back(cluster);
  _numClusters++;
}

void Track::addMatchedCluster(Cluster* cluster)
{
  _matchedClusters.push_back(cluster);
  _numMatchedClusters++;
}

Cluster* Track::getCluster(unsigned int n) const
{
  assert(n < getNumClusters() && "Track: cluster index exceeds vector range");
  return _clusters.at(n);
}

Cluster* Track::getMatchedCluster(unsigned int n) const
{
  assert(n < getNumMatchedClusters() && "Track: matched cluster index exceeds vector range");
  return _matchedClusters.at(n);
}

Track::Track() :
  _originX(0), _originY(0), _originErrX(0), _originErrY(0),
  _slopeX(0), _slopeY(0), _slopeErrX(0), _slopeErrY(0),
  _covarianceX(0), _covarianceY(0), _chi2(0), _numClusters(0),
  _numMatchedClusters(0), _index(-1)
{ }

Track::Track(const Track& old)
{
  this->_originX = old._originX;
  this->_originY = old._originY;
  this->_originErrX = old._originErrX;
  this->_originErrY = old._originErrY;
  this->_slopeX = old._slopeX;
  this->_slopeY = old._slopeY;
  this->_slopeErrX = old._slopeErrX;
  this->_slopeErrY = old._slopeErrY;
  this->_covarianceX = old._covarianceX;
  this->_covarianceY = old._covarianceY;
  this->_chi2 = old._chi2;
  this->_clusters = old._clusters;
  this->_numClusters = old._numClusters;
  this->_matchedClusters = old._matchedClusters;
  this->_numMatchedClusters = old._numMatchedClusters;
  this->_index = old._index;
}

}
