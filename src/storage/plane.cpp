#include "plane.h"

#include <cassert>
#include <vector>
#include <iostream>

#include "hit.h"
#include "cluster.h"

using std::cout;
using std::endl;

namespace Storage {

void Plane::print()
{
  cout << "\nPLANE:\n"
       << "  Address: " << this << "\n"
       << "  Num hits: " << getNumHits() << "\n"
       << "  Num clusters: " << getNumClusters() << endl;

  for (unsigned int ncluster = 0; ncluster < getNumClusters(); ncluster++)
    getCluster(ncluster)->print();

  for (unsigned int nhit = 0; nhit < getNumHits(); nhit++)
    getHit(nhit)->print();
}

void Plane::addHit(Hit* hit)
{
  _hits.push_back(hit);
  hit->_plane = this;
  _numHits++;
}

void Plane::addCluster(Cluster* cluster)
{
  _clusters.push_back(cluster);
  cluster->_plane = this;
  _numClusters++;
}

Hit* Plane::getHit(unsigned int n) const
{
  assert(n < getNumHits() && "Plane: hit index exceeds vector size");
  return _hits.at(n);
}

Cluster* Plane::getCluster(unsigned int n) const
{
  assert(n < getNumClusters() && "Plane: cluster index exceeds vector size");
  return _clusters.at(n);
}

Plane::Plane(unsigned int planeNum) :
  _planeNum(planeNum), _numHits(0), _numClusters(0)
{ }

}
