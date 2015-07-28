#ifndef PLANE_H
#define PLANE_H

#include <vector>

namespace Storage {

class Hit;
class Cluster;

/*******************************************************************************
 * Plane class contains the hits and clusters for one sensor plane as well as
 * a plane number.
 */

class Plane
{
private:
  unsigned int _planeNum;
  unsigned int _numHits;
  std::vector<Hit*> _hits;
  unsigned int _numClusters;
  std::vector<Cluster*> _clusters;

protected:
  void addHit(Hit* hit);
  void addCluster(Cluster* cluster);
  Plane(unsigned int planeNum);
  ~Plane() { ; }

public:
  Hit* getHit(unsigned int n) const;
  Cluster* getCluster(unsigned int n) const;

  void print();

  inline unsigned int getPlaneNum() const { return _planeNum; }
  inline unsigned int getNumHits() const { return _numHits; }
  inline unsigned int getNumClusters() const { return _numClusters; }

  friend class Event;
};

}

#endif // PLANE_H
