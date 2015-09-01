#include "synchronizerms.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <math.h>
#include <string>

#include <Rtypes.h>
#include <TH1D.h>
#include <TCanvas.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../mechanics/alignment.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../processors/processors.h"
#include "../processors/clustermaker.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;
using std::flush;
using std::cin;

namespace Loopers {


void SynchronizeRMS::displaySyncPlots(TH1D* syncHist, TH1D* unsyncHist)
{
  const Int_t thresholdBin = unsyncHist->FindBin(0);
  const double eventsInThreholdBin =
      unsyncHist->GetBinContent(thresholdBin);
  const double thresholdRangeToBin =
      2 * _threshold / unsyncHist->GetBinWidth(thresholdBin);
  const double eventsInThresholdRange =
      eventsInThreholdBin * thresholdRangeToBin;

  const double missRate = eventsInThresholdRange /
      unsyncHist->Integral(0, unsyncHist->GetNbinsX() + 1);

  cout << "Expected desynchronization miss rate: (" << missRate
       << ")^" << _preDiscards + 1 << endl;
  cout << endl;

  TCanvas* can = new TCanvas("SyncCanvas", "Synchroniation Fluctuations", 1200, 600);
  can->Divide(2, 1);
  can->cd(1);
  syncHist->Draw();
  can->cd(2);
  unsyncHist->Draw();
  can->Update();
  can->WaitPrimitive();
}

unsigned int SynchronizeRMS::syncRatioLoop(ULong64_t start, ULong64_t num,
                                        unsigned int refOffset, unsigned int dutOffset)
{
  Storage::Event* refStart = _refStorage->readEvent(start + refOffset);
  Storage::Event* refEnd = _refStorage->readEvent(start + num + refOffset);
  Storage::Event* dutStart = _dutStorage->readEvent(start + dutOffset);
  Storage::Event* dutEnd = _dutStorage->readEvent(start + num + dutOffset);

  return 0.;
}

bool SynchronizeRMS::isSync(Analyzers::DUTCorrelation *correlation)
{
  double offsetX = 0, sigmaX = 0, ratioX=0.0, intX=0.0;
  double offsetY = 0, sigmaY = 0, ratioY=0.0, intY=0.0;
  // analyze the offset data
  for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsensor);

    //if the device has one pixel in x direction
    if ( _dutDevice->getSensor(nsensor)->getNumX() == 1 )
    {
      TH1D* alignPadX = correlation->getAlignmentPlotX(nsensor);
      offsetX = 0;
      sigmaX = 0;
      int maxbinX=0;
      double SFoffsetX=0;
      Processors::fitGaussian( alignPadX, offsetX, sigmaX,_displayDistributions);
      maxbinX= alignPadX->GetMaximumBin();
      SFoffsetX=(alignPadX->GetXaxis()->GetBinUpEdge(maxbinX) + alignPadX->GetXaxis()->GetBinLowEdge(maxbinX))/2.0;
      offsetX = - SFoffsetX;
      intX = alignPadX->Integral(0,alignPadX->GetNbinsX()+2);
      if(intX>0.0)
	ratioX=alignPadX->GetBinContent(maxbinX)/intX;
      std::cout << "    X offset " << offsetX << " RMS: " << alignPadX->GetRMS() << " Fit width: " << sigmaX << " max bin Content: " << alignPadX->GetBinContent(maxbinX) << " ratio: " << ratioX << std::endl;
    }
    //if the device has one pixel in x direction
    if ( _dutDevice->getSensor(nsensor)->getNumY() == 1 )
    {
      TH1D* alignPadY = correlation->getAlignmentPlotY(nsensor);
      offsetY = 0;
      sigmaY = 0;
      int maxbinY=0;
      double SFoffsetY=0;
      Processors::fitGaussian( alignPadY, offsetY, sigmaY,_displayDistributions);
      maxbinY= alignPadY->GetMaximumBin();
      SFoffsetY=(alignPadY->GetXaxis()->GetBinUpEdge(maxbinY) + alignPadY->GetXaxis()->GetBinLowEdge(maxbinY))/2.0;      
      offsetY = - SFoffsetY;
      intY = alignPadY->Integral(0,alignPadY->GetNbinsX()+2);
      if(intY>0.0)
	ratioY=alignPadY->GetBinContent(maxbinY)/intY;
      
      std::cout << "    Y offset " << offsetY << " RMS: " << alignPadY->GetRMS() << " Fit width: " << sigmaY << " max bin Content: " << alignPadY->GetBinContent(maxbinY) << " ratio: " << ratioY << std::endl;
    }
  }

  return (ratioX>_threshold);
}

void SynchronizeRMS::loop()
{
  if (_bufferSize - _preDiscards <= 2)
    throw "SynchronizeRMS: buffer size and pre-discards make it impossible to sync";

  // If the devices don't have a ratio, caucluate it
  //if (_dutDevice->getSyncRatio() <= 0 || _refDevice->getSyncRatio() <= 0)
  //  calculateSyncRatio();

  // Processors::SynchronizeRMSr sync(_refDevice, _dutDevice, _threshold, _bufferSize);
  unsigned int refShift = 0;
  unsigned int dutShift = 0;

  // Some statistics for reporting
  ULong64_t singleRefOffsets = 0;
  ULong64_t multipleRefOffsets = 0;
  ULong64_t singleDutOffsets = 0;
  ULong64_t multipleDutOffsets = 0;
  ULong64_t questionableEvents = 0;
  ULong64_t failedSyncs = 0;
  ULong64_t goodSyncs = 0;
  ULong64_t numConsecutiveFails = 0;
  ULong64_t invalidEvents = 0;
  ULong64_t invalidEventsDUT = 0;
  ULong64_t invalidEventsRef = 0;  
  ULong64_t totalReadEvents = 0;
  ULong64_t totalWrittenEvents = 0;
  ULong64_t numConsecutiveSyncs = 0;
  ULong64_t consecutiveFails = 0;
  ULong64_t consecutiveSyncs = 0;

  // align for beginning events to remove.
  //....fill in

  // next we have a starting point that is aligned
  // Make the synchronization plots to see if this sample is synchronized
  int evtBlock = 30000;
  unsigned int maxIterations = int(float(_endEvent - _startEvent)/float(evtBlock)) + 1;
  std::cout << "maxIterations: " << maxIterations << std::endl;
  for(unsigned iter = 0; iter < maxIterations; ++iter){
    Analyzers::DUTCorrelation *correlation = new Analyzers::DUTCorrelation(_refDevice, _dutDevice, 0);
  
    for (unsigned int nevent = _startEvent+evtBlock*iter; nevent <= std::min(_startEvent+evtBlock*iter+evtBlock,_endEvent); ++nevent)
      {
	if(numConsecutiveFails<2) // make sure this is not a second pass for synchro
	  ++totalReadEvents;
	
	// read in events
	Storage::Event* refEvent = _refStorage->readEvent(nevent);
	Storage::Event* dutEvent = _dutStorage->readEvent(nevent+_eventSkip-1);    
	if (refEvent->getNumClusters() || dutEvent->getNumClusters())
	  throw "SynchronizeRMS: can't recluster an event, mask the tree in the input";
	for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
	  _clusterMaker->generateClusters(refEvent, nplane);
	for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
	  _clusterMaker->generateClusters(dutEvent, nplane);

	// applies the alignment to the newly created clusters
	Processors::applyAlignment(refEvent, _refDevice);
	Processors::applyAlignment(dutEvent, _dutDevice);

	if(dutEvent->getInvalid()){
	  std::cout << "Invalid Event DUT - skipping" << std::endl;
	  ++invalidEventsDUT;
	  continue;
	}

	if( refEvent->getInvalid()){
	  std::cout << "Invalid Event Reference - skipping " << std::endl;
	  ++invalidEventsRef;
	  continue;
	}
	
	// fill the correlation hists
	correlation->processEvent(refEvent, dutEvent);

	// clean up
	delete refEvent;
	delete dutEvent; 
      } // end block
    
    // writing out the events that are synchronized
    std::cout << "event shift: " << _eventSkip << std::endl;
    if(isSync(correlation)){
      std::cout << "SYNC------event shift: " << _eventSkip << std::endl;
      // reset variables
      numConsecutiveFails=0;
      ++goodSyncs;
      
      for (unsigned int nevent = _startEvent+evtBlock*iter; nevent <= std::min(_startEvent+evtBlock*iter+evtBlock,_endEvent); ++nevent)
	{
	  
	  // read in events
	  Storage::Event* refEvent = _refStorage->readEvent(nevent );
	  Storage::Event* dutEvent = _dutStorage->readEvent(nevent +_eventSkip-1);
	  if(dutEvent->getInvalid()){
	    std::cout << "Invalid Event DUT - skipping writing" << std::endl;
	    continue;
	  }
	  
	  if( refEvent->getInvalid()){
	    std::cout << "Invalid Event Reference - skipping writing " << std::endl;
	    continue;
	  }
	  
	  _refOutput->writeEvent(refEvent);
	  _dutOutput->writeEvent(dutEvent);
	  totalWrittenEvents++;
	  
	  // clean up
	  delete refEvent;
	  delete dutEvent; 
	}
    } else if(numConsecutiveFails==0) {      
      ++numConsecutiveFails; // removed data
      invalidEvents+=std::min(unsigned(_endEvent - (_startEvent+evtBlock*iter)), unsigned(evtBlock));
    }
    else if(numConsecutiveFails<_maxOffset) {
      ++numConsecutiveFails;
      --iter;
      ++_eventSkip;
    }else{
      ++failedSyncs;
      numConsecutiveFails=0; // could not sync
      // reset the eventSkip in case the thresholds were too tight
      _eventSkip-=(_maxOffset-1);
      invalidEvents+=std::min(unsigned(_endEvent - (_startEvent+evtBlock*iter)), unsigned(evtBlock));
    }
    
    //progressBar(iter*evtBlock);
    
    delete correlation;
  }//

  // check the end synch...by starting from the end
  
  if (VERBOSE)
  {
    cout << "\nSYNCHRONIZE STATISTICS:\n";
    cout << "  REF offsets of 1     : " << singleRefOffsets << "\n";
    cout << "  REF offsets > 1      : " << multipleRefOffsets << "\n";
    cout << "  DUT offsets of 1     : " << singleDutOffsets << "\n";
    cout << "  DUT offsets > 1      : " << multipleDutOffsets << "\n";
    cout << "  Questionable events  : " << questionableEvents << "\n";
    cout << "  Good syncs           : " << goodSyncs << "\n";
    cout << "  Consecutive syncs    : " << numConsecutiveSyncs << "\n";
    cout << "  Failed syncs         : " << failedSyncs << "\n";
    cout << "  Consecutive fails    : " << numConsecutiveFails << "\n";
    cout << "  Invalid events       : " << invalidEvents << "\n";
    cout << "  Invalid events DUT   : " << invalidEventsDUT << "\n";
    cout << "  Invalid events Ref   : " << invalidEventsRef << "\n";        
    cout << "  Total read events    : " << totalReadEvents << "\n";
    cout << "  Total written events : " << totalWrittenEvents << "\n";
    cout << flush;
  }
}

void SynchronizeRMS::setThreshold(double threshold) { _threshold = threshold; }
void SynchronizeRMS::setSyncSample(unsigned int value) { _syncRatioSample = value; }
void SynchronizeRMS::setMaxOffset(unsigned int value) { _maxOffset = value; }
void SynchronizeRMS::setMaxLargeSyncAttempts(unsigned int value) { _maxLargeSyncAttempts = value; }
void SynchronizeRMS::setBufferSize(unsigned int value) { _bufferSize = value; }
void SynchronizeRMS::setPreDiscards(unsigned int value) { _preDiscards = value; }
void SynchronizeRMS::setMaxConsecutiveFails(unsigned int value) { _maxConsecutiveFails = value; }
void SynchronizeRMS::setDisplayDistributions(bool value) { _displayDistributions = value; }
  
SynchronizeRMS::SynchronizeRMS(Mechanics::Device* refDevice,
                         Mechanics::Device* dutDevice,
			 Processors::ClusterMaker* clusterMaker,
                         Storage::StorageIO* refOutput,
                         Storage::StorageIO* dutOutput,
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
  _syncRatioSample(100),
  _maxOffset(100),
  _maxLargeSyncAttempts(20),
  _bufferSize(10),
  _preDiscards(2),
  _displayDistributions(false),
  _maxConsecutiveFails(3)
{
  assert(refInput && dutInput && clusterMaker && refOutput && dutOutput &&
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         dutInput->getNumPlanes() == dutDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");  
}

}
