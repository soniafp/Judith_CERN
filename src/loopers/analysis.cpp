#include "analysis.h"

#include <cassert>
#include <vector>

#include <Rtypes.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"

namespace Loopers {

void Analysis::loop()
{
  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent);

    for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
      _singleAnalyzers.at(i)->processEvent(refEvent);

    progressBar(nevent);

    delete refEvent;
  }

  for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
    _singleAnalyzers.at(i)->postProcessing();
}

Analysis::Analysis(Storage::StorageIO* refInput,
                   ULong64_t startEvent,
                   ULong64_t numEvents,
                   unsigned int eventSkip) :
  Looper(refInput, 0, startEvent, numEvents, eventSkip)
{
  assert(refInput && "Looper: initialized with null object(s)");
}

}
