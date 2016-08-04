#include "coarsealigndut.h"

#include <cassert>
#include <iostream>
#include <vector>

#include <Rtypes.h>
#include <TAxis.h>

//added Matevz 20141130
#include <TCanvas.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../mechanics/alignment.h"
#include "../processors/processors.h"
#include "../processors/clustermaker.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../analyzers/dutcorrelation.h"

namespace Loopers {

void CoarseAlignDut::loop()
{
  Analyzers::DUTCorrelation correlation(_refDevice, _dutDevice, 0);

  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent+_eventSkip-1);
    Storage::Event* dutEvent = _dutStorage->readEvent(nevent);

    if (refEvent->getNumClusters() || dutEvent->getNumClusters())
      throw "CoarseAlignDut: can't recluster an event, mask the tree in the input";
    //std::cout << "telescope" << std::endl;
    for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
      _clusterMaker->generateClusters(refEvent, nplane);
    //std::cout << "DUT----" << std::endl;    
    for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
      _clusterMaker->generateClusters(dutEvent, nplane);

    Processors::applyAlignment(refEvent, _refDevice);
    Processors::applyAlignment(dutEvent, _dutDevice);
    //std::cout << "clustering"  << nevent << std::endl;
    correlation.processEvent(refEvent, dutEvent);

    progressBar(nevent);

    delete refEvent;
    delete dutEvent;
  }


  for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsensor);
    std::cout << "CoarseAlignDut::loop - sensor name: " << sensor->getName() << std::endl;
    //S.F: TH1D* alignX = correlation.getAlignmentPlotX(nsensor);
    double offsetX = 0;
    double sigmaX = 0;
    //S.F: Processors::fitGaussian(alignX, offsetX, sigmaX, _displayFits);

    //S.F: TH1D* alignY = correlation.getAlignmentPlotY(nsensor);
    double offsetY = 0;
    double sigmaY = 0;
    //S.F: Processors::fitGaussian(alignY, offsetY, sigmaY, _displayFits);

    //Matevz 20141202 ------------- coarse alignment of pad and strip detectors
    //if the device has one pixel in x direction
    if ( _dutDevice->getSensor(nsensor)->getNumX() == 1 )
    {
      std::cout << "the device has 1 pixel in x direction. "<< std::endl;
      TH1D* alignPadX = correlation.getPadAlignmentPlotX(nsensor);
      offsetX = 0;
      sigmaX = 0;
      int maxbinX=0;
      double SFoffsetX=0;
      Processors::fitGaussian( alignPadX, offsetX, sigmaX,_displayFits);
      maxbinX= alignPadX->GetMaximumBin();
      //SFoffsetX=alignPadX->GetBinContent(maxbinX);
      SFoffsetX=(alignPadX->GetXaxis()->GetBinUpEdge(maxbinX) + alignPadX->GetXaxis()->GetBinLowEdge(maxbinX))/2.0;
      offsetX = - SFoffsetX;
      //Processors::fitBox( alignPadX, offsetX, sigmaX,
      //                    sensor->getPitchX(),
      //                    _displayFits);
      //offsetX = - offsetX;
      std::cout << "X offset " << offsetX << " RMS: " << alignPadX->GetRMS() << " Fit width: " << sigmaX << std::endl;
    }
    //if the device has one pixel in y direction
    if ( _dutDevice->getSensor(nsensor)->getNumY() == 1 )
    {
      std::cout << "the device has 1 pixel in Y direction. " << std::endl;
      TH1D* alignPadY = correlation.getPadAlignmentPlotY(nsensor);
      offsetY = 0;
      sigmaY = 0;
      int maxbinY=0;
      double SFoffsetY=0;
      Processors::fitGaussian( alignPadY, offsetY, sigmaY,_displayFits);
      maxbinY= alignPadY->GetMaximumBin();
      //SFoffsetY=alignPadY->GetBinCenter(maxbinY);
      SFoffsetY=(alignPadY->GetXaxis()->GetBinUpEdge(maxbinY) + alignPadY->GetXaxis()->GetBinLowEdge(maxbinY))/2.0;      
      std::cout << "getbin_center " << SFoffsetY<< std::endl;
      offsetY = - SFoffsetY;
      
      //Processors::fitBox( alignPadY, offsetY, sigmaY,
      //                    sensor->getPitchY(),
      //                    _displayFits);
      //offsetY = -offsetY;
      
      std::cout << "Y offset " << offsetY << " RMS: " << alignPadY->GetRMS() << " Fit width: " << sigmaY << std::endl;
    } else{
      TH1D* alignX = correlation.getAlignmentPlotX(nsensor);
      Processors::fitGaussian(alignX, offsetX, sigmaX, _displayFits);
      TH1D* alignY = correlation.getAlignmentPlotY(nsensor);
      Processors::fitGaussian(alignY, offsetY, sigmaY, _displayFits);
    }
    //Add another TH1D for single-pixel coarse alignment
    //add another processor for fitting a gaussian to the new th1d.
    std::cout << "X offset " << offsetX  << std::endl;
    std::cout << "Y offset " << offsetY  << std::endl;
    std::cout << "X current " << sensor->getOffX() << std::endl;
    std::cout << "X new     " << sensor->getOffX() - offsetX << std::endl;
    std::cout << "Y current " << sensor->getOffY() << std::endl;
    std::cout << "Y new     " << sensor->getOffY() - offsetY << std::endl;
    sensor->setOffX(sensor->getOffX() - offsetX);
    sensor->setOffY(sensor->getOffY() - offsetY);
  }

  _dutDevice->getAlignment()->writeFile();
}

void CoarseAlignDut::setDisplayFits(bool value) { _displayFits = value; }

CoarseAlignDut::CoarseAlignDut(Mechanics::Device* refDevice,
                               Mechanics::Device* dutDevice,
                               Processors::ClusterMaker* clusterMaker,
                               Storage::StorageIO* refInput,
                               Storage::StorageIO* dutInput,
                               ULong64_t startEvent,
                               ULong64_t numEvents,
                               Long64_t eventSkip) :
  Looper(refInput, dutInput, startEvent, numEvents, eventSkip),
  _refDevice(refDevice),
  _dutDevice(dutDevice),
  _clusterMaker(clusterMaker),
  _displayFits(true)
{
  assert(refInput && dutInput && refDevice && dutDevice && clusterMaker &&
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         dutInput->getNumPlanes() == dutDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
