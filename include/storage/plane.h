#ifndef PLANE_H
#define PLANE_H

#include <vector>

namespace Storage {

class Hit;
class Cluster;

/**
  * Collection of hits and clusters from the same sensor plane. Note that the
  * hits and clusters are actually managed by the `Event` and provided here
  * only for navigation.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Plane {
protected:
  /** Index of this plane within the list of sensor planes in the storage */
  const size_t m_planeNum;
  /** List of hits in this plane for an event */
  std::vector<Hit*> m_hits;
  /** List of clusters in this plane for an event */
  std::vector<Cluster*> m_clusters;

  /** Only constructed by an `Event` object */
  Plane(size_t nplane);
  /** Only destructed by an `Event` object */
  ~Plane() {}

  /** Clear values so the object can be re-used */
  void clear();

public:
  /** Print hit information to standard output */
  void print();

  inline Hit& getHit(size_t n) const { return *m_hits[n]; }
  inline Cluster& getCluster(size_t n) const { return *m_clusters[n]; }

  inline size_t getPlaneNum() const { return m_planeNum; }
  inline size_t getNumHits() const { return m_hits.size(); }
  inline size_t getNumClusters() const { return m_clusters.size(); }

  friend class Event;
};

}

#endif // PLANE_H
