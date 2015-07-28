#include "dutdepiction.h"

#include <cassert>
#include <sstream>
#include <math.h>
#include <vector>

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>

// Access to the device being analyzed and its sensors
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
// Access to the data stored in the event
#include "../storage/hit.h"
#include "../storage/cluster.h"
#include "../storage/plane.h"
#include "../storage/track.h"
#include "../storage/event.h"
// Some generic processors to calcualte typical event related things
#include "../processors/processors.h"
#include "../processors/eventdepictor.h"
// This header defines all the cuts
#include "cuts.h"

namespace Analyzers {

void DUTDepictor::processEvent(const Storage::Event* refEvent,
                               const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);

  if (_depictEvent)
  {
    // Check if the event passes the cuts
    for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
      if (!_eventCuts.at(ncut)->check(refEvent)) return;

    _depictor->depictEvent(refEvent, dutEvent);
  }

  if (_depictClusters)
  {
    std::vector<const Storage::Cluster*> refClusters;
    for (unsigned int ncluster = 0; ncluster < refEvent->getNumClusters(); ncluster++)
    {
      const Storage::Cluster* cluster = refEvent->getCluster(ncluster);
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster)) continue;
      refClusters.push_back(cluster);
    }

    std::vector<const Storage::Cluster*> dutClusters;
    for (unsigned int ncluster = 0; ncluster < dutEvent->getNumClusters(); ncluster++)
    {
      const Storage::Cluster* cluster = dutEvent->getCluster(ncluster);
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster)) continue;
      dutClusters.push_back(cluster);
    }

    _depictor->depictClusters(refClusters, dutClusters);
  }

  if (_depictTracks)
    {
    for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
    {
      Storage::Track* track = refEvent->getTrack(ntrack);

      for (unsigned int ncut = 0; ncut < _numTrackCuts; ncut++)
        if (!_trackCuts.at(ncut)->check(track)) continue;

      _depictor->depictTrack(track);
    }
  }
}

void DUTDepictor::postProcessing() { } // Needs to be declared even if not used

DUTDepictor::DUTDepictor(const Mechanics::Device* refDevice,
                         const Mechanics::Device* dutDevice,
                         TDirectory* dir,
                         const char* suffix,
                         bool depictEvent,
                         bool depictClusters,
                         bool depictTracks,
                         double zoom) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix),
  _depictEvent(depictEvent),
  _depictClusters(depictClusters),
  _depictTracks(depictTracks),
  _depictor(0)
{
  _depictor = new Processors::EventDepictor(_refDevice, _dutDevice);
  _depictor->setZoom(zoom);
}

}
