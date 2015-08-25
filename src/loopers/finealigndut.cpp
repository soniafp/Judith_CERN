#include "finealigndut.h"

#include <cassert>
#include <vector>
#include <iostream>

#include <Rtypes.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../mechanics/alignment.h"
#include "../processors/processors.h"
#include "../processors/clustermaker.h"
#include "../processors/trackmaker.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../analyzers/cuts.h"
#include "../analyzers/dutresiduals.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

namespace Loopers {

void FineAlignDut::loop()
{
  for (unsigned int niter = 0; niter < _numIterations; niter++)
  {
    cout << "Iteration " << niter << " of " << _numIterations - 1 << endl;

    // Use broad residual resolution for the first iteration
    const unsigned int numPixX = (niter == 0) ? _numPixXBroad : _numPixX;
    const double binxPerPix = (niter == 0) ? _binsPerPixBroad : _binsPerPix;

    // Residuals of DUT clusters to ref. tracks
    Analyzers::DUTResiduals residuals(_refDevice, _dutDevice, 0, "", numPixX, binxPerPix, _numBinsY);

    // Use events with only 1 track
    Analyzers::Cuts::EventTracks* cut1 =
        new Analyzers::Cuts::EventTracks(1, Analyzers::EventCut::EQ);

    // Use tracks with one hit in each plane
    const unsigned int numClusters = _refDevice->getNumSensors();
    Analyzers::Cuts::TrackClusters* cut2 =
        new Analyzers::Cuts::TrackClusters(numClusters, Analyzers::TrackCut::EQ);

    // Use tracks with low chi2
//    Analyzers::Cuts::TrackClusters* cut3 =
//        new Analyzers::Cuts::TrackChi2(1, Analyzers::TrackCut::LT);

    // Note: the analyzer will delete the cuts
    residuals.addCut(cut1);
    residuals.addCut(cut2);

    for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
    {
      Storage::Event* refEvent = _refStorage->readEvent(nevent);
      Storage::Event* dutEvent = _dutStorage->readEvent(nevent);

      if (refEvent->getNumClusters() || dutEvent->getNumClusters())
        throw "FineAlignDut: can't recluster an event, mask the tree in the input";
      for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
        _clusterMaker->generateClusters(refEvent, nplane);
      for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
        _clusterMaker->generateClusters(dutEvent, nplane);

      Processors::applyAlignment(refEvent, _refDevice);
      Processors::applyAlignment(dutEvent, _dutDevice);

      if (refEvent->getNumTracks())
        throw "FineAlign: can't re-track an event, mask the tree in the input";
      _trackMaker->generateTracks(refEvent,
                                  _refDevice->getBeamSlopeX(),
                                  _refDevice->getBeamSlopeY());

      residuals.processEvent(refEvent, dutEvent);

      progressBar(nevent);

      delete refEvent;
      delete dutEvent;
    }

    for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
    {
      Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);

      double offsetX = 0, offsetY = 0, rotation = 0;
      Processors::residualAlignment(residuals.getResidualXY(nsens),
                                    residuals.getResidualYX(nsens),
                                    offsetX, offsetY, rotation, _displayFits);

      sensor->setOffX(sensor->getOffX() + offsetX);
      sensor->setOffY(sensor->getOffY() + offsetY);
      sensor->setRotZ(sensor->getRotZ() + rotation);
    }
  }

  _dutDevice->getAlignment()->writeFile();
}

void FineAlignDut::setNumIteratioins(unsigned int value) { _numIterations = value; }
void FineAlignDut::setNumBinsY(unsigned int value) { _numBinsY = value; }
void FineAlignDut::setNumPixX(unsigned int value) { _numPixX = value; }
void FineAlignDut::setBinsPerPix(double value) { _binsPerPix = value; }
void FineAlignDut::setNumPixXBroad(unsigned int value) { _numPixXBroad = value; }
void FineAlignDut::setBinsPerPixBroad(double value) { _binsPerPixBroad = value; }
void FineAlignDut::setDisplayFits(bool value) { _displayFits = value; }
void FineAlignDut::setRelaxation(double value) { _relaxation = value; }

FineAlignDut::FineAlignDut(Mechanics::Device* refDevice,
                           Mechanics::Device* dutDevice,
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
  _clusterMaker(clusterMaker),
  _trackMaker(trackMaker),
  _numIterations(5),
  _numBinsY(15),
  _numPixX(5),
  _binsPerPix(10),
  _numPixXBroad(20),
  _binsPerPixBroad(1),
  _displayFits(true),
  _relaxation(0.8)
{
  assert(refInput && dutInput && refDevice && dutDevice && clusterMaker && trackMaker &&
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         dutInput->getNumPlanes() == dutDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
