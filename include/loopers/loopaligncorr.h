#ifndef LOOPALIGNCORR
#define LOOPALIGNCORR

#include <vector>

#include "loopers/looper.h"

namespace Mechanics { class Device; }

namespace Loopers {

/**
  * Loop over all events in a series of inputs. Build up the inter-plane 
  * correlation of clusters, by plotting the spatial difference of all clusters
  * on consecutive planes. Use the result to align the planes.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopAlignCorr : public Looper {
private:
  std::vector<Mechanics::Device*> m_devices;
  /** For each plane, gives the global index (device + plane) of the plane
    * relative to which it will be correlated */
  std::vector<size_t> m_planeMap;

public:
  /** Object must be constructed with an output */
  LoopAlignCorr(const std::vector<Mechanics::Device*>& devices);
  ~LoopAlignCorr() {}

  /** Redefined loop checks that a single input is given */
  void loop();
  void execute();
};

}

#endif  // LOOPALIGNCORR

