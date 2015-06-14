#ifndef LOOPALIGNCORR
#define LOOPALIGNCORR

#include <vector>

#include "analyzers/clusterresiduals.h"
#include "loopers/looper.h"

namespace Storage { class StorageI; }
namespace Mechanics { class Device; }

namespace Loopers {

/**
  * Loop over all events in a series of inputs. Build up the inter-plane 
  * residuals of clusters with the cluster residuals analyzer. Use the result
  * to align the planes.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopAlignCorr : public Looper {
private:
  /** Analyzer to store the computations for aligning */
  Analyzers::ClusterResiduals m_residuals;

public:
  LoopAlignCorr(
      const std::vector<Storage::StorageI*>& inputs,
      const std::vector<Mechanics::Device*>& devices);
  LoopAlignCorr(
      Storage::StorageI& input,
      Mechanics::Device& device);
  ~LoopAlignCorr() {}

  /** Compute and apply alignment as post-processing step */
  void finalize();
};

}

#endif  // LOOPALIGNCORR

