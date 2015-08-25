#include "examplelooper.h"

#include <cassert>
#include <vector>

#include <Rtypes.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../processors/processors.h"
#include "../processors/clustermaker.h"
#include "../processors/trackmaker.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"

namespace Loopers {

void ExampleLooper::loop()
{
  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent);
    Storage::Event* dutEvent = _dutStorage->readEvent(nevent);

    if (refEvent->getNumClusters() == 0)
      for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
        _clusterMaker->generateClusters(refEvent, nplane);
    if (dutEvent->getNumClusters() == 0)
      for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
        _clusterMaker->generateClusters(dutEvent, nplane);

    Processors::applyAlignment(refEvent, _refDevice);
    Processors::applyAlignment(dutEvent, _dutDevice);

    if (refEvent->getNumTracks() == 0)
      _trackMaker->generateTracks(refEvent,
                                  _refDevice->getBeamSlopeX(),
                                  _refDevice->getBeamSlopeY());
    if (dutEvent->getNumTracks() == 0)
      _trackMaker->generateTracks(dutEvent,
                                  _dutDevice->getBeamSlopeX(),
                                  _dutDevice->getBeamSlopeY());

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

ExampleLooper::ExampleLooper(Mechanics::Device* refDevice,
                             Mechanics::Device* dutDevice,
                             Storage::StorageIO* refOutput,
                             Storage::StorageIO* dutOutput,
                             Processors::ClusterMaker* clusterMaker,
                             Processors::TrackMaker* trackMaker,
                             Storage::StorageIO* refInput,
                             Storage::StorageIO* dutInput,
                             ULong64_t startEvent,
                             ULong64_t numEvents,
                             Long64_t eventSkip) :
  Looper(refInput, dutInput, startEvent, numEvents, eventSkip),
  _refDevice(refDevice),
  _dutDevice(dutDevice),
  _refOutput(refOutput),
  _dutOutput(dutOutput),
  _clusterMaker(clusterMaker),
  _trackMaker(trackMaker)
{
  assert(refInput && dutInput && refDevice && dutDevice && refOutput &&
         dutOutput && clusterMaker && trackMaker &&
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         dutInput->getNumPlanes() == dutDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
