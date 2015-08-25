#include "finealign.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <algorithm>

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
#include "../analyzers/residuals.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

namespace Loopers {

void FineAlign::loop()
{
  // Build a vector of sensor indices which will be permutated at each iteration
  std::vector<unsigned int> sensorPermutations(_refDevice->getNumSensors(), 0);
  for (unsigned int i = 0; i < _refDevice->getNumSensors(); i++)
    sensorPermutations[i] = i;
  // Start with permutation so that the first itertation permutes back to the
  // ordered list (just looks less strange if the first iteration is ordered)
  std::prev_permutation(sensorPermutations.begin(), sensorPermutations.end());

  for (unsigned int niter = 0; niter < _numIterations+2; niter++)
  {
    cout << "Iteration " << niter << " of " << _numIterations - 1 << endl;

    // Get the average slopes to make device rotations
    double avgSlopeX = 0.0;
    double avgSlopeY = 0.0;
    ULong64_t numSlopes = 0;

    // Permute the order in which the sensors will be processed
    std::next_permutation(sensorPermutations.begin(), sensorPermutations.end());
    if(niter==_numIterations){
      sensorPermutations[0]=5;
      sensorPermutations[1]=4;
      sensorPermutations[2]=3;
      sensorPermutations[3]=2;
      sensorPermutations[4]=1;
      sensorPermutations[5]=0;      
    }
    if(niter==_numIterations+1){
      sensorPermutations[0]=4;
      sensorPermutations[1]=2;
      sensorPermutations[2]=0;
      sensorPermutations[3]=3;
      sensorPermutations[4]=5;
      sensorPermutations[5]=1;      
    }    
    // Print the permutation
    for(unsigned j=0; j<sensorPermutations.size(); ++j){ std::cout << sensorPermutations.at(j) << " ";}
    cout << std::endl;
    
    // Each sensor gets an unbiased residual run, and there is an extra run for overall alignment
    for (unsigned int nsensor = 0; nsensor < _refDevice->getNumSensors(); nsensor++)
    {
      // Use sensor index from the permutated list of indices
      const unsigned int nsens = sensorPermutations[nsensor];

      cout << "Sensor " << nsens << endl;

      Mechanics::Sensor* sensor = _refDevice->getSensor(nsens);

      // Use broad residual resolution for the first iteration
      const unsigned int numPixX = (niter == 0) ? _numPixXBroad : _numPixX;
      const double binxPerPix = (niter == 0) ? _binsPerPixBroad : _binsPerPix;

      Analyzers::Residuals residuals(_refDevice, 0, "", numPixX, binxPerPix, _numBinsY); // create residual plots

      // Use events with only 1 track
      Analyzers::Cuts::EventTracks* cut1 =
          new Analyzers::Cuts::EventTracks(1, Analyzers::Cut::EQ);

      // Use tracks with one hit in each plane (one is masked)
      const unsigned int numClusters = _refDevice->getNumSensors() - 1;
      Analyzers::Cuts::TrackClusters* cut2 =
          new Analyzers::Cuts::TrackClusters(numClusters, Analyzers::Cut::EQ);

      // Note: the analyzer will delete the cuts
      residuals.addCut(cut1);
      residuals.addCut(cut2);

      // iterate events
      for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
      {
        Storage::Event* refEvent = _refStorage->readEvent(nevent);

        if (refEvent->getNumClusters())
          throw "FineAlign: can't recluster an event, mask the tree in the input";
        for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
          _clusterMaker->generateClusters(refEvent, nplane); // make clusters in the plane

        Processors::applyAlignment(refEvent, _refDevice);

        if (refEvent->getNumTracks())
          throw "FineAlign: can't re-track an event, mask the tree in the input";
        _trackMaker->generateTracks(refEvent,
                                    _refDevice->getBeamSlopeX(),
                                    _refDevice->getBeamSlopeY(),
                                    nsens); // This is the masked plane, which is looped over.

        // For the average track slopes
        for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
        {
          Storage::Track* track = refEvent->getTrack(ntrack);
          avgSlopeX += track->getSlopeX();
          avgSlopeY += track->getSlopeY();
          numSlopes++;
        }

        residuals.processEvent(refEvent);

        progressBar(nevent);

        delete refEvent;
      }

      double offsetX = 0, offsetY = 0, rotation = 0;
      Processors::residualAlignment(residuals.getResidualXY(nsens),
                                    residuals.getResidualYX(nsens),
                                    offsetX, offsetY, rotation, 
                                    _relaxation, _displayFits);

      sensor->setOffX(sensor->getOffX() + offsetX);
      sensor->setOffY(sensor->getOffY() + offsetY);
      sensor->setRotZ(sensor->getRotZ() + rotation);
    }

    // Adjust the device rotation using the average slopes
    avgSlopeX /= (double)numSlopes;
    avgSlopeY /= (double)numSlopes;
    _refDevice->setBeamSlopeX(_refDevice->getBeamSlopeX() + avgSlopeX);
    _refDevice->setBeamSlopeY(_refDevice->getBeamSlopeY() + avgSlopeY);

    cout << endl; // Space between iterations
  }

  _refDevice->getAlignment()->writeFile();
}

void FineAlign::setNumIteratioins(unsigned int value) { _numIterations = value; }
void FineAlign::setNumBinsY(unsigned int value) { _numBinsY = value; }
void FineAlign::setNumPixX(unsigned int value) { _numPixX = value; }
void FineAlign::setBinsPerPix(double value) { _binsPerPix = value; }
void FineAlign::setNumPixXBroad(unsigned int value) { _numPixXBroad = value; }
void FineAlign::setBinsPerPixBroad(double value) { _binsPerPixBroad = value; }
void FineAlign::setDisplayFits(bool value) { _displayFits = value; }
void FineAlign::setRelaxation(double value) { _relaxation = value; }

FineAlign::FineAlign(Mechanics::Device* refDevice,
                     Processors::ClusterMaker* clusterMaker,
                     Processors::TrackMaker* trackMaker,
                     Storage::StorageIO* refInput,
                     ULong64_t startEvent,
                     ULong64_t numEvents,
                     Long64_t eventSkip) :
  Looper(refInput, 0, startEvent, numEvents, eventSkip),
  _refDevice(refDevice),
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
  assert(refInput && refDevice && clusterMaker && trackMaker &&
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
