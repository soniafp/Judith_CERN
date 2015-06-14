#ifndef LOOPALIGNTRACKS
#define LOOPALIGNTRACKS

#include <vector>

#include "processors/tracking.h"
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
private:

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

