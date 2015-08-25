#ifndef ANALYSISDUT_H
#define ANALYSISDUT_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class TrackMatcher; }

namespace Loopers {

class AnalysisDut : public Looper
{
private:
  Processors::TrackMatcher* _trackMatcher;

public:
  AnalysisDut(/* These arguments are needed to be passed to the base looper class */
              Storage::StorageIO* refInput,
              Storage::StorageIO* dutInput,
              /* Use if the looper needs to make clusters and/or tracks... */
              Processors::TrackMatcher* trackMatcher,
              ULong64_t startEvent = 0,
              ULong64_t numEvents = 0,
              Long64_t eventSkip = 1);

  void loop();
};

}

#endif
