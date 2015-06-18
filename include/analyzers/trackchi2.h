#ifndef ANA_TRACKCHI2
#define ANA_TRACKCHI2

#include <string>
#include <vector>
#include <list>

#include "analyzers/analyzer.h"

namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }

namespace Analyzers {

/**
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class TrackChi2 : public Analyzer {
public:
  // For fast chi^2, don't want to loop over full Track and Cluster objects,
  // instead define these light weight versions which can be stored in
  // continuous memory

  /** Cluster information required for chi^2 alignment */
  struct Cluster {
    /** Pointer to the sensor which aligns this cluster */
    const Mechanics::Sensor* sensor;
    /** Cluster location in local pixel coordinates */
    double pixX;
    double pixY;
    double pixErrX;
    double pixErrY;
    Cluster(
        const Mechanics::Sensor& sensor, 
        double pixX,
        double pixY,
        double pixErrX,
        double pixErrY) :
        sensor(&sensor),
        pixX(pixX),
        pixY(pixY),
        pixErrX(pixErrX),
        pixErrY(pixErrY) {}
  };

  /** Track information required for chi^2 alignment */
  struct Track {
    // Note: don't use pointers since the clusters can be move dinto continous
    // memory. Isntead, keep their index which should remain consistent.
    /** Index to constituent clusters in cluster list */
    std::vector<size_t> constituents;
    /** Index to matching clusters in cluster list */
    std::vector<size_t> matches;
    Track(size_t nRefPlanes, size_t nDUTPlanes) :
        constituents(),
        matches() {
      // Don't want memeory allocation at each cluster addition
      constituents.reserve(nRefPlanes);
      matches.reserve(nDUTPlanes);
    }
  };

private:
  TrackChi2(const TrackChi2&);
  TrackChi2& operator=(const TrackChi2&);

  /** List of all tracks built */
  std::list<Track> m_tracks;
  /** List of all clusters in the tracks built */
  std::list<Cluster> m_clusters;
  /** Keep track of cluster index for last added */
  size_t m_icluster;

  /** Number of planes in the reference device */
  size_t m_numRefPlanes;
  /** Number of planes in the DUT device */
  size_t m_numDUTPlanes;

  /** Constructor calls this to initialize memory */
  void initialize();

  /** Base virtual method defined, gives code to run at each loop */
  void process();

public:
  /** Automatically calls the correct base constuctor */
  template <class T>
  TrackChi2(const T& t) :
      Analyzer(t),
      m_icluster(0),
      m_numRefPlanes(0),
      m_numDUTPlanes(0) {
    initialize();
  }
  ~TrackChi2() {}

  void setOutput(TDirectory* dir, const std::string& name="TrackChi2") {
    // Just adds the default name
    Analyzer::setOutput(dir, name);
  }

  const std::list<Track>& getTracks() const { return m_tracks; }
  const std::list<Cluster>& getClusters() const { return m_clusters; }
};

}

#endif  // ANA_TRACKCHI2 

