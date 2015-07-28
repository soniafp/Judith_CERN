#include "processevents.h"

#include <cassert>
#include <vector>
#include <iostream>

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

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;
using std::flush;

namespace Loopers {

void ProcessEvents::loop()
{
  // Some statistics for reporting
  ULong64_t statProcessedEvents = 0;
  ULong64_t statGeneratedClusters = 0;
  ULong64_t statGeneratedTracks = 0;
  ULong64_t statNoClusterEvents = 0;
  ULong64_t statNoTrackEvents = 0;
  ULong64_t statSingleTrackEvents = 0;
  ULong64_t statMostClustersEvent = 0;
  unsigned int statMostClusters = 0;
  ULong64_t statMostTracksEvent = 0;
  unsigned int statMostTracks = 0;

  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent);
    
    if (refEvent->getNumClusters())
      throw "ProcessEvents: can't recluster an event, mask the tree in the input";
    for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
      if (_clusterMaker) _clusterMaker->generateClusters(refEvent, nplane);

    Processors::applyAlignment(refEvent, _refDevice);

    if (refEvent->getNumTracks())
      throw "ProcessEvents: can't re-track an event, mask the tree in the input";
    //HP
    if (_trackMaker) _trackMaker->generateTracks(refEvent,
                                                 _refDevice->getBeamSlopeX(),
                                                 _refDevice->getBeamSlopeY(),2);

    // Write the event
    _refOutput->writeEvent(refEvent);

    for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
      _singleAnalyzers.at(i)->processEvent(refEvent);

    // Fill the statistics
    statProcessedEvents++;
    statGeneratedClusters += refEvent->getNumClusters();
    statGeneratedTracks += refEvent->getNumTracks();
    if (!refEvent->getNumClusters())
      statNoClusterEvents++;
    if (!refEvent->getNumTracks())
      statNoTrackEvents++;
    if (refEvent->getNumTracks() == 1)
      statSingleTrackEvents++;
    if (nevent == _startEvent || refEvent->getNumClusters() > statMostClusters)
    {
      statMostClusters = refEvent->getNumClusters();
      statMostClustersEvent = nevent;
    }
    if (nevent == _startEvent || refEvent->getNumTracks() > statMostTracks)
    {
      statMostTracks = refEvent->getNumTracks();
      statMostTracksEvent = nevent;
    }

    progressBar(nevent);

    delete refEvent;
  }

  for (unsigned int i = 0; i < _numSingleAnalyzers; i++)
    _singleAnalyzers.at(i)->postProcessing();
  for (unsigned int i = 0; i < _numDualAnalyzers; i++)
    _dualAnalyzers.at(i)->postProcessing();

  if (VERBOSE)
  {
    cout << "\nPROCESS RUN STATISTICS:\n";
    cout << "  Requested events:      " << _numEvents << "\n";
    cout << "  Processed events:      " << statProcessedEvents << "\n";
    cout << "  Generated clusters:    " << statGeneratedClusters << "\n";
    cout << "  Generated tracks:      " << statGeneratedTracks << "\n";
    cout << "  Clusters per event:    " << (double)statGeneratedClusters /
            (double)statProcessedEvents << "\n";
    cout << "  Clusters per sensor:   " << (double)statGeneratedClusters /
            (double)statProcessedEvents / (double)_refDevice->getNumSensors() << "\n";
    cout << "  Tracks per event:      " << (double)statGeneratedTracks /
            (double)statProcessedEvents << "\n";
    cout << "  No cluster events:     " << statNoClusterEvents /
            (double)statProcessedEvents * 100 << "%\n";
    cout << "  No track events:       " << statNoTrackEvents /
            (double)statProcessedEvents * 100 << "%\n";
    cout << "  Single track events:   " << statSingleTrackEvents /
            (double)statProcessedEvents * 100 << "%\n";
    cout << "  Most clusters (event): " << statMostClusters << " (" <<
            statMostClustersEvent << ")\n";
    cout << "  Most Tracks (event):   " << statMostTracks << " (" <<
            statMostTracksEvent << ")\n";
    cout << flush;
  }
}

ProcessEvents::ProcessEvents(Mechanics::Device* refDevice,
                             Storage::StorageIO* refOutput,
                             Processors::ClusterMaker* clusterMaker,
                             Processors::TrackMaker* trackMaker,
                             Storage::StorageIO* refInput,
                             ULong64_t startEvent,
                             ULong64_t numEvents,
                             unsigned int eventSkip) :
  Looper(refInput, 0, startEvent, numEvents, eventSkip),
  _refDevice(refDevice),
  _refOutput(refOutput),
  _clusterMaker(clusterMaker),
  _trackMaker(trackMaker)
{
  assert(refInput && refDevice && refOutput &&
         "Looper: initialized with null object(s)");
  std::cout << "ProcessEvents::ProcessEvents - Planes: " << refInput->getNumPlanes()
  << " sensors: " << refDevice->getNumSensors() << std::endl;
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
