#ifndef LOOPALIGNCORR
#define LOOPALIGNCORR

#include <vector>

#include "analyzers/correlations.h"
#include "loopers/looper.h"

namespace Storage { class StorageI; }
namespace Mechanics { class Device; }

namespace Loopers {

/**
  * Loop over all events in a series of inputs. Build up the inter-plane 
  * correlation of clusters with the correlations analyzer. Use the result
  * to align the planes.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopAlignCorr : public Looper {
private:
  /** Devices to post-process alignment into */
  std::vector<Mechanics::Device*> m_devices;
  /** Analyzer to store the computations for aligning */
  Analyzers::Correlations m_correlations;

public:
  LoopAlignCorr(
      const std::vector<Storage::StorageI*>& inputs,
      const std::vector<Mechanics::Device*>& devices);
  /** Convenience single device constructor */
  LoopAlignCorr(
      Storage::StorageI& input,
      Mechanics::Device& device);
  ~LoopAlignCorr() {}

  /** Compute and apply alignment as post-processing step */
  void finalize();
};

}

#endif  // LOOPALIGNCORR

