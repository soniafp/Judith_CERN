#ifndef COARSEALIGNDUT_H
#define COARSEALIGNDUT_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }

namespace Loopers {

class CoarseAlignDut : public Looper
{
private:
  Mechanics::Device* _refDevice;
  Mechanics::Device* _dutDevice;
  Processors::ClusterMaker* _clusterMaker;

  bool _displayFits;

public:
  CoarseAlignDut(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
                 Mechanics::Device* refDevice,
                 Mechanics::Device* dutDevice,
                 /* Use if the looper needs to make clusters and/or tracks... */
                 Processors::ClusterMaker* clusterMaker,
                 /* These arguments are needed to be passed to the base looper class */
                 Storage::StorageIO* refInput,
                 Storage::StorageIO* dutInput,
                 ULong64_t startEvent = 0,
                 ULong64_t numEvents = 0,
                 Long64_t eventSkip = 1);

  void loop();

  void setCorrPeakWidth(double value);
  void setDisplayFits(bool value);
};

}

#endif
