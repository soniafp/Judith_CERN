#ifndef LOOPALIGNTRACKS
#define LOOPALIGNTRACKS

#include <vector>
#include <list>

#include <Math/IFunction.h>

#include "processors/tracking.h"
#include "analyzers/trackchi2.h"
#include "loopers/looper.h"

namespace Storage { class StorageI; }
namespace Mechanics { class Device; }

namespace Loopers {

/**
  * Align device planes based on track residuals.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopAlignTracks : public Looper {
public:
  /** Deep within the belly of the beast, lies an interface to for a
    * multidimensional function with a method to compute gradients */
  class Chi2Minimizer : public ROOT::Math::IBaseFunctionMultiDim {
  private:
    Mechanics::Device* m_device;
    std::vector<Analyzers::TrackChi2::Cluster> m_clusters;
    std::vector<Analyzers::TrackChi2::Track> m_tracks;
    const unsigned m_ndim;

    double DoEval(const double* x) const;

  public:
    Chi2Minimizer(
      Mechanics::Device& device,
      const std::list<Analyzers::TrackChi2::Cluster>& clusters,
      const std::list<Analyzers::TrackChi2::Track>& tracklets) :
      // Copying over the light weight objects needed for alignment. The tracks
      // refere to clusters by index, so the order is preserved.
      m_device(&device),
      m_clusters(clusters.begin(), clusters.end()),
      m_tracks(tracklets.begin(), tracklets.end()),
      m_ndim(m_device->getNumSensors()*6) {}

    ROOT::Math::IBaseFunctionMultiDim* Clone() const;
    inline unsigned int NDim() const { return m_ndim; }
  };

private:
  /** Analyzer computes track residuals for each event. */
  Analyzers::TrackChi2 m_trackChi2;

public:
  /** Tracking processor to generate the tracks for alignment */
  Processors::Tracking m_tracking;

  LoopAlignTracks(
      const std::vector<Storage::StorageI*>& inputs,
      const std::vector<Mechanics::Device*>& devices);
  LoopAlignTracks(
      Storage::StorageI& input,
      Mechanics::Device& device);
  ~LoopAlignTracks() {}

  /** Override default execute behaviour */
  void execute();

  /** Compute and apply alignment as post-processing step */
  void finalize();
};

}

#endif  // LOOPALIGNTRACKS

