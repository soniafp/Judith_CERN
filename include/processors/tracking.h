#ifndef PROC_TRACKING_H
#define PROC_TRACKING_H

#include <list>

#include "processors/processor.h"

namespace Storage { class Hit; }
namespace Storage { class Cluster; }
namespace Storage { class Track; }
namespace Storage { class Event; }

namespace Processors {

/**
  * Runs a tracking algorithm on events from a device.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Tracking : public Processor {
protected:
  /** The number of planes used to build tracks */
  const size_t m_nplanes;
  /** The RMS of distances of clusters from one plane to another. The structure
    * is iplane1*nplanes + iplane2 (yields scale from plane1 to plane2) */
  std::vector<double> m_transitionsX;
  std::vector<double> m_transitionsY;

  /** Base virtual method called at each loop iteration */
  virtual void process();

public:
  /** Search radius, in sigma, from the last cluster */
  double m_radius;
  /** Minimum number of clusters to make a track. Below 3 is ignored. */
  size_t m_minClusters;

  /** Tracking requires no device information, and can be done for one device
    * at a time only */
  Tracking(size_t nplanes)  :
      Processor(1),  // single device processor
      m_nplanes(nplanes),
      m_transitionsX(m_nplanes*m_nplanes, 1),
      m_transitionsY(m_nplanes*m_nplanes, 1),
      m_radius(5),
      m_minClusters(3) {}
  virtual ~Tracking() {}

  void setTransitionX(size_t from, size_t to, double scale);
  void setTransitionY(size_t from, size_t to, double scale);

  /** Algorithm builds the `Track` object, by performing a straight line
    * fit to the clusters in the track. */
  static void buildTrack(
      Storage::Track& track,
      const std::list<Storage::Cluster*>& clusters);
};

}

#endif  // PROC_TRACKING_H
