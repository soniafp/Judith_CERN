#ifndef COARSEALIGN_H
#define COARSEALIGN_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }

namespace Loopers {

class CoarseAlign : public Looper
{
private:
  Mechanics::Device* _refDevice;
  Processors::ClusterMaker* _clusterMaker;

  bool _displayFits; // Choose whether or not to display the fits

public:
  CoarseAlign(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
              Mechanics::Device* refDevice,
              /* Use if the looper needs to make clusters and/or tracks... */
              Processors::ClusterMaker* clusterMaker,
              /* These arguments are needed to be passed to the base looper class */
              Storage::StorageIO* refInput,
              ULong64_t startEvent = 0,
              ULong64_t numEvents = 0,
              Long64_t eventSkip = 1);

  void loop();

  void setCorrPeakWidth(double value);
  void setDisplayFits(bool value);
};

}

#endif
