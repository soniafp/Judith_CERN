#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <list>

namespace Storage { class Hit; }
namespace Storage { class Cluster; }
namespace Storage { class Event; }

namespace Processors {

/**
  * Class runs a clustering algorithm on events. It can be configured to
  * get the desired result from the algorithm. It can also be extended, to make
  * more elaborate algorithms.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Clustering {
protected:
  /** Algorithm builds a list of hits belonging to the same cluster as the
    * provided seed. It is called from `process` and can be extended to
    * implement a different clustering algorithm */
  virtual void clusterSeed(
      Storage::Hit& seed,
      std::list<Storage::Hit*>& hits,
      std::list<Storage::Hit*>& clustered);
  /** Algorithm buidls the `Cluster` object, and computes its pixel values from
    * the given list of clustered hits. Can be extended to achieve different 
    * mean and uncertainties for the clusters. */
  virtual void buildCluster(
      Storage::Cluster& cluster,
      std::list<Storage::Hit*>& clustered);

public:
  /** Maximal number of rows between two hits to cluster */
  unsigned m_maxRows;
  /** Maximal number of columns between two hits to cluster */
  unsigned m_maxCols;
  /** Weight the cluster center and RMS by its hit values */
  bool m_weighted;

  Clustering();
  virtual ~Clustering() {}

  /** Loop over the planes in an event, and keep providing seeds to the
    * clustering algorithm until all hits are exhausted. */
  virtual void process(Storage::Event& event);
};

}

#endif  // CLUSTERING_H
