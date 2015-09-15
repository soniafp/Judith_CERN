#ifndef PROC_CLUSTERING_H
#define PROC_CLUSTERING_H

#include <list>

#include "processors/processor.h"

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
class Clustering : public Processor {
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

  /** Processing is done device-by-device, so make single device method */
  virtual void processEvent(Storage::Event& event);

  /** Base virtual method called at each loop iteration */
  virtual void process();

public:
  /** Maximal number of rows between two hits to cluster */
  unsigned m_maxRows;
  /** Maximal number of columns between two hits to cluster */
  unsigned m_maxCols;
  /** Weight the cluster center and RMS by its hit values */
  bool m_weighted;

  /** Clustering doesn't require device information, so construct only with
    * the expected number of devices (events) */
  Clustering(size_t ndevices) : 
      Processor(ndevices),
      m_maxRows(1),
      m_maxCols(1),
      m_weighted(false) {}
  /** Keep the default constructor around, to make single device clustering */
  Clustering()  :
      Processor(1),
      m_maxRows(1),
      m_maxCols(1),
      m_weighted(false) {}
  virtual ~Clustering() {}
};

}

#endif  // PROC_CLUSTERING_H
