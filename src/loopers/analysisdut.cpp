#include "analysisdut.h"

#include <cassert>
#include <vector>

#include <Rtypes.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../processors/trackmatcher.h"

namespace Loopers {

void AnalysisDut::loop()
{
  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent);
    Storage::Event* dutEvent = _dutStorage->readEvent(nevent);

    // Match ref tracks to dut clusters (information stored in event)
    _trackMatcher->matchEvent(refEvent, dutEvent);

    for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
      _singleAnalyzers.at(i)->processEvent(refEvent);
    for (unsigned int i = 0; i < _numDualAnalyzers; i++)
      _dualAnalyzers.at(i)->processEvent(refEvent, dutEvent);

    progressBar(nevent);

    delete refEvent;
    delete dutEvent;
  }

  for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
    _singleAnalyzers.at(i)->postProcessing();
  for (unsigned int i = 0; i < _numDualAnalyzers; i++)
    _dualAnalyzers.at(i)->postProcessing();
}

AnalysisDut::AnalysisDut(Storage::StorageIO* refInput,
                         Storage::StorageIO* dutInput,
                         Processors::TrackMatcher* trackMatcher,
                         ULong64_t startEvent,
                         ULong64_t numEvents,
                         Long64_t eventSkip) :
  Looper(refInput, dutInput, startEvent, numEvents, eventSkip),
  _trackMatcher(trackMatcher)
{
  assert(refInput && dutInput && trackMatcher && "Looper: initialized with null object(s)");
}

}
