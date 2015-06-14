#ifndef LOOPTRANSFERS
#define LOOPTRANSFERS

#include <vector>

#include "analyzers/clusterresiduals.h"
#include "loopers/looper.h"

namespace Storage { class StorageI; }
namespace Mechanics { class Device; }
namespace Processors { class Tracking; }

namespace Loopers {

/**
  * Run the ClusterResiduals analyzer on events to meausre the transfer
  * distributions between all planes in many devices. Set the transfers in
  * a tracking processor using those measurements.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopTransfers : public Looper {
private:
  /** Analyzer to store the computations for aligning */
  Analyzers::ClusterResiduals m_residuals;

  /** Vector of transfer scales for each sensor pair as computed in the
    * residuals analyzer */
  std::vector<double> m_scalesX;
  std::vector<double> m_scalesY;

public:
  LoopTransfers(
      const std::vector<Storage::StorageI*>& inputs,
      const std::vector<Mechanics::Device*>& devices);
  LoopTransfers(
      Storage::StorageI& input,
      Mechanics::Device& device);
  ~LoopTransfers() {}

  /** Compute and apply alignment as post-processing step */
  void finalize();

  void apply(Processors::Tracking& tracking);
};

}

#endif  // LOOPTRANFERS

