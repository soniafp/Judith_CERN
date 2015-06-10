#ifndef LOOPPROCESS_H
#define LOOPPROCESS_H

#include "loopers/looper.h"

namespace Storage { class StorageI; }
namespace Storage { class StorageO; }
namespace Processors { class Clustering; }
namespace Processors { class Aligning; }

namespace Loopers {

/**
  * Loop over all events in an input, and write it back to an output. The
  * events are processed to generate clusters and/or tracks (or neither),
  * so the output will contain these objects. Also applies alignment if
  * requested.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class LoopProcess : public Looper {
private:
  /** Output file where to store the processed events */
  Storage::StorageO& m_output;

public:
  /** If a clustering algorithm is given, it will be applied */
  Processors::Clustering* m_clustering; 
  /** If an aligning algorithm is given, it will be applied */
  Processors::Aligning* m_aligning;

  /** Object must be constructed with an output */
  LoopProcess(Storage::StorageI& input, Storage::StorageO& output);
  ~LoopProcess() {}

  /** Redefined loop checks that a single input is given */
  void loop();
  void execute();
};

}

#endif  // LOOPPROCESS_H

