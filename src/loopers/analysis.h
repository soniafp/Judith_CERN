#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }

namespace Loopers {

class Analysis : public Looper
{
public:
  Analysis(/* These arguments are needed to be passed to the base looper class */
           Storage::StorageIO* refInput,
           ULong64_t startEvent = 0,
           ULong64_t numEvents = 0,
           unsigned int eventSkip = 1);

  void loop();
};

}

#endif
