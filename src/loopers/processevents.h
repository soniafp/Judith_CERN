#ifndef PROCESSEVENTS_H
#define PROCESSEVENTS_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }

namespace Loopers {

class ProcessEvents : public Looper
{
private:
  Mechanics::Device* _refDevice;
  Storage::StorageIO* _refOutput;
  Processors::ClusterMaker* _clusterMaker;
  Processors::TrackMaker* _trackMaker;

public:
  ProcessEvents(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
                Mechanics::Device* refDevice,
                /* Use if you will use the input to generate a new output (processing) */
                Storage::StorageIO* refOutput,
                /* Use if the looper needs to make clusters and/or tracks... */
                Processors::ClusterMaker* clusterMaker,
                Processors::TrackMaker* trackMaker,
                /* These arguments are needed to be passed to the base looper class */
                Storage::StorageIO* refInput,
                ULong64_t startEvent = 0,
                ULong64_t numEvents = 0,
                unsigned int eventSkip = 1);

  void loop();
};

}

#endif
