#include "synchronize.h"

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
#include "../mechanics/alignment.h"
#include "../processors/synchronizer.h"
#include "../processors/largesynchronizer.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../analyzers/syncfluctuation.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;
using std::flush;
using std::cin;

namespace Loopers {

Analyzers::SyncFluctuation* Synchronize::setupAnalyzer()
{
  // Get the RMS of the event time differences to scale unsync
  const unsigned int rmsSample = 10;

  if (_totalEvents < rmsSample)
    throw "Synchronize: too few events to do anything";

  double changeRMS = 0;
  ULong64_t lastTimeStamp = 0, lastTimeStampDUT=0;
  for (unsigned int nevent = 0; nevent < rmsSample; nevent++)
  {
    Storage::Event* event = _refStorage->readEvent(nevent);
    Storage::Event* dutevent = _dutStorage->readEvent(nevent);
    const double diff = (event->getTimeStamp() - lastTimeStamp) /
        (double)_refDevice->getReadOutWindow() / (double)_refDevice->getClockRate();

  std::cout << "Telescope start: " << event->getTimeStamp() << " end: " << lastTimeStamp
	    << " DUT: " <<_refDevice->getReadOutWindow() << " " <<_refDevice->getClockRate()
	    << " diff: " << (event->getTimeStamp() - lastTimeStamp) << std::endl;
    
  std::cout << "DUT start: " << dutevent->getTimeStamp() << " end: " << lastTimeStampDUT
	    << " DUT: " <<_dutDevice->getReadOutWindow() << " " <<_dutDevice->getClockRate()
	    << " diff: " << (dutevent->getTimeStamp() - lastTimeStampDUT) << std::endl;
    
    if (nevent > 0)
      changeRMS += pow(diff, 2);
    lastTimeStamp = event->getTimeStamp();
    lastTimeStampDUT = dutevent->getTimeStamp();    
    delete event;
  }

  changeRMS /= (double)rmsSample;
  changeRMS = sqrt(changeRMS);

  Analyzers::SyncFluctuation* fluctuations
      = new Analyzers::SyncFluctuation(_refDevice, _dutDevice, 0, "",
                                       _threshold, 2 * changeRMS);

  return fluctuations;
}

void Synchronize::displaySyncPlots(TH1D* syncHist, TH1D* unsyncHist)
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

unsigned int Synchronize::syncRatioLoop(ULong64_t start, ULong64_t num,
                                        unsigned int refOffset, unsigned int dutOffset)
{
  Storage::Event* refStart = _refStorage->readEvent(start + refOffset);
  Storage::Event* refEnd = _refStorage->readEvent(start + num + refOffset);
  Storage::Event* dutStart = _dutStorage->readEvent(start + dutOffset);
  Storage::Event* dutEnd = _dutStorage->readEvent(start + num + dutOffset);

  const double refToDut = (refEnd->getTimeStamp() - refStart->getTimeStamp()) /
      (double)(dutEnd->getTimeStamp() - dutStart->getTimeStamp());
  std::cout << "Telescope start: " << refStart->getTimeStamp() << " end: " << dutEnd->getTimeStamp()
	    << " DUT: " << dutEnd->getTimeStamp() << " " << dutStart->getTimeStamp()
	    << " diff: " << (refEnd->getTimeStamp() - refStart->getTimeStamp())<< std::endl;
  _dutDevice->setSyncRatio(refToDut);
  _refDevice->setSyncRatio(1.0);

  delete refStart;
  delete refEnd;
  delete dutStart;
  delete dutEnd;

  // Make the synchronization plots to see if this sample is synchronized
  Analyzers::SyncFluctuation* fluctuations = setupAnalyzer();

  if (start + num + (refOffset > dutOffset ? refOffset : dutOffset) > _endEvent)
    throw "Synchronize: asked for an synchronize ratio loop which exceeds the actual run";

  // Loop over some events to see how the synchronziation parameters hold up
  for (ULong64_t nevent = start; nevent < start + num; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent + refOffset);
    Storage::Event* dutEvent = _dutStorage->readEvent(nevent + dutOffset);

    if (refEvent->getInvalid() || dutEvent->getInvalid())
    {
      delete refEvent;
      delete dutEvent;
      continue;
    }

    fluctuations->processEvent(refEvent, dutEvent);

    delete refEvent;
    delete dutEvent;
  }

  TH1D* syncHist = fluctuations->getSynchronized();
  TH1D* unsyncHist = fluctuations->getUnsynchronized();

  const unsigned int totalOverflow =
      syncHist->GetBinContent(0) + syncHist->GetBinContent(syncHist->GetNbinsX() + 1);

  if (totalOverflow)
  {
    cout << "Synchronized plot overflow of " << totalOverflow << " detected.\n";
    cout << "Possible desynchronization at event " << _syncRatioSample - totalOverflow << endl;
  }

  // Get some information from these synchronization plots
  if (_displayDistributions) displaySyncPlots(syncHist, unsyncHist);

  delete fluctuations; // Delete AFTER SHOWING THE PLOTS!!

  return totalOverflow;
}

void Synchronize::calculateSyncRatio()
{
  if (_totalEvents <= _syncRatioSample)
    throw "Synchronize: needs more events than the initial sample size";

  // Process some number of starting events
  ULong64_t preStart = 0;
  ULong64_t preNum = _syncRatioSample;
  unsigned int preRefOffset = 0;
  unsigned int preDutOffset = 0;
  unsigned int desyncEvent = 0;

  desyncEvent = syncRatioLoop(preStart, preNum, preRefOffset, preDutOffset);

  if (desyncEvent)
    throw "Synchronize: initial desynchronization, synchronize with another file first";

  _dutDevice->getAlignment()->writeFile();
  _refDevice->getAlignment()->writeFile();
}

bool Synchronize::findLargeOffset(unsigned int nevent,
                                  unsigned int& refShift,
                                  unsigned int& dutShift)
{
  // Start at nevent + 1 in case the desync was at nevent
  nevent += 1;

  // This is how far this will loop
  if (nevent + _bufferSize + _maxOffset > _endEvent) return true;

  // Use one event to initialize the previous time stamps
  const Storage::Event* refInitial = _refStorage->readEvent(nevent);
  const Storage::Event* dutInitial = _dutStorage->readEvent(nevent);
  ULong64_t initialRefTime = refInitial->getTimeStamp();
  ULong64_t initialDutTime = dutInitial->getTimeStamp();
  delete refInitial;
  delete dutInitial;

  // Don't use this last event
  nevent += 1;

  // Generate large offset processors for the case where the ref is static and
  // the dut is offset, and vice versa
  Processors::LargeSynchronizer refSync(_refDevice, _dutDevice, _threshold,
                                        _bufferSize, _refDevice->getReadOutWindow(),
                                        initialRefTime, initialDutTime);
  Processors::LargeSynchronizer dutSync(_dutDevice, _refDevice, _threshold,
                                        _bufferSize, _refDevice->getReadOutWindow(),
                                        initialDutTime, initialRefTime);

  // Fill the two buffers
  for (unsigned int n = 0; n < _bufferSize; n++)
  {
    const Storage::Event* refEvent = _refStorage->readEvent(nevent + n);
    const Storage::Event* dutEvent = _dutStorage->readEvent(nevent + n);
    refSync.addEvents(refEvent, dutEvent);
    dutSync.addEvents(dutEvent, refEvent);
  }

  // Check if the buffers agree, if so no sync was needed
  if (refSync.checkBuffer() && dutSync.checkBuffer()) return true;

  // The remaining events are shifting the non-static device
  for (unsigned int n = 0; n < _maxOffset; n++)
  {
    const Storage::Event* refEvent = _refStorage->readEvent(nevent + _bufferSize + n);
    const Storage::Event* dutEvent = _dutStorage->readEvent(nevent + _bufferSize + n);
    refSync.addEvents(refEvent, dutEvent);
    dutSync.addEvents(dutEvent, refEvent);

    if (refSync.checkBuffer()) { refShift += n + 1; return true; }
    if (dutSync.checkBuffer()) { dutShift += n + 1; return true; }
  }

  return false;
}

void Synchronize::loop()
{
  if (_bufferSize - _preDiscards <= 2)
    throw "Synchronize: buffer size and pre-discards make it impossible to sync";

  // If the devices don't have a ratio, caucluate it
  if (_dutDevice->getSyncRatio() <= 0 || _refDevice->getSyncRatio() <= 0)
    calculateSyncRatio();

  Processors::Synchronizer sync(_refDevice, _dutDevice, _threshold, _bufferSize);
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
  ULong64_t totalReadEvents = 0;
  ULong64_t totalWrittenEvents = 0;
  ULong64_t totalBufferNotFull = 0;  
  ULong64_t numConsecutiveSyncs = 0;
  ULong64_t numLargeSyncs = 0;
  ULong64_t totalLargeSyncOffsets = 0;
  ULong64_t consecutiveFails = 0;
  ULong64_t consecutiveSyncs = 0;

  // Make the synchronization plots to see if this sample is synchronized
  Analyzers::SyncFluctuation* fluctuations = setupAnalyzer();

  for (unsigned int nevent = _startEvent; nevent <= _endEvent; nevent += _eventSkip)
  {
    // Do some cleaning of nevent due to offsets
    const unsigned int largeOffset = (refShift > dutShift) ? refShift : dutShift;
    const unsigned int smallOffset = (refShift > dutShift) ? dutShift : refShift;

    if (nevent + largeOffset > _endEvent) break;
    if (smallOffset > 0)
    {
      nevent += smallOffset;
      refShift -= smallOffset;
      dutShift -= smallOffset;
    }

    Storage::Event* refEvent = _refStorage->readEvent(nevent + refShift);
    Storage::Event* dutEvent = _dutStorage->readEvent(nevent + dutShift);

    totalReadEvents++;

    if (refEvent->getInvalid() || dutEvent->getInvalid())
    {
      delete refEvent;
      delete dutEvent;
      invalidEvents++;
      continue;
    }

    // Sync will delete the events when it's done storing them in buffer
    sync.processEvent(refEvent, dutEvent);

    /* Synchronize when the buffers are full, there is a desync in the
     * buffer at the desired position (to allow discarding preceeding events)
     * and there are more than one desynchronized events in the buffers (this
     * avoids resynchronizing the entire buffer for one buggy event). */
    if (sync.getFull() && sync.getNumDiff() > 1 &&
        sync.getOffsetToDesync() <= (int)_preDiscards)
    {
      unsigned int desync = 0, refOffsetCurrent = 0, dutOffsetCurrent = 0;
      const bool status = sync.calculateOffset(desync, refOffsetCurrent, dutOffsetCurrent);
      consecutiveSyncs++;

      if (status)
      {
        refShift += refOffsetCurrent;
        dutShift += dutOffsetCurrent;

        goodSyncs++;

        if (refOffsetCurrent == 1) singleRefOffsets++;
        else if (refOffsetCurrent > 1) multipleRefOffsets++;
        else if (dutOffsetCurrent == 1) singleDutOffsets++;
        else if (dutOffsetCurrent > 1) multipleDutOffsets++;

        if (consecutiveFails > 1)
          numConsecutiveFails++;
        consecutiveFails = 0;
      }
      else
      {
        failedSyncs++;
        consecutiveFails++;

        // Large offset mode in case of many consecutive fails
        if (consecutiveFails > _maxConsecutiveFails)
        {
          if (VERBOSE)
            cout << "Synchronize: WARN :: Synchronize: max consectuvie fails reached, "
                    "attempting to recover in large offset mode" << endl;

          numLargeSyncs++;
          const ULong64_t prevRefShift = refShift;
          const ULong64_t prevDutShift = dutShift;

          // Try large resyncs, offset by buffer size between attempts to get over trouble area
          unsigned int nattempts = 0;
          while (!findLargeOffset(nevent, refShift, dutShift))
          {
            nattempts++;
            nevent += _bufferSize;

            // Reset the shifts (the llast find large offset changed them but not correctly)
            refShift = prevRefShift;
            dutShift = prevDutShift;

            if (nattempts >= _maxLargeSyncAttempts)
            {
              if (VERBOSE) cout << "Synchronize: WARN :: Max large sync offsets reached, "
                                   "the rest of the file won't be processed";
              nevent = _endEvent + 1; // Ends the loops but still prints the stats
            }
          }

          numLargeSyncs += refShift - prevRefShift + dutShift - prevDutShift;
        }
      }

      sync.clearBuffers(); // All events in the buffers are lost and can't be written to output
    }

    // If the buffers are full, output the last event in the buffers
    if (sync.getFull())
    {
      assert(sync.getRefTargetEvent() && sync.getDutTargetEvent() &&
             "Synchronization: has empty events when it shouldn't");

      if (consecutiveSyncs > 1)
        numConsecutiveSyncs++;
      consecutiveSyncs = 0;

      if (sync.getTargetDiff() > _threshold)
      {
        questionableEvents++;
      }
      else
      {
        Storage::Event* refEvent = sync.getRefTargetEvent();
        Storage::Event* dutEvent = sync.getDutTargetEvent();
        _refOutput->writeEvent(sync.getRefTargetEvent());
        _dutOutput->writeEvent(sync.getDutTargetEvent());
        fluctuations->processEvent(refEvent, dutEvent);
        totalWrittenEvents++;
      }
    }else{
      std::cout << "NOT writting " << std::endl;
      ++totalBufferNotFull;
    }

    progressBar(nevent);
  }

  cout << endl; // Progress bar never finishes

  TH1D* syncHist = fluctuations->getSynchronized();
  TH1D* unsyncHist = fluctuations->getUnsynchronized();

  // Get some information from these synchronization plots
  if (_displayDistributions) displaySyncPlots(syncHist, unsyncHist);

  delete fluctuations; // Delete AFTER SHOWING THE PLOTS!!

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
    cout << "  Large syncs          : " << numLargeSyncs << "\n";
    cout << "  Large sync offsets   : " << totalLargeSyncOffsets << "\n";
    cout << "  Total read events    : " << totalReadEvents << "\n";
    cout << "  Total written events : " << totalWrittenEvents << "\n";
    cout << "  Total NOT written events : " << totalBufferNotFull << "\n";    
    cout << flush;
  }
}

void Synchronize::setThreshold(double threshold) { _threshold = threshold; }
void Synchronize::setSyncSample(unsigned int value) { _syncRatioSample = value; }
void Synchronize::setMaxOffset(unsigned int value) { _maxOffset = value; }
void Synchronize::setMaxLargeSyncAttempts(unsigned int value) { _maxLargeSyncAttempts = value; }
void Synchronize::setBufferSize(unsigned int value) { _bufferSize = value; }
void Synchronize::setPreDiscards(unsigned int value) { _preDiscards = value; }
void Synchronize::setMaxConsecutiveFails(unsigned int value) { _maxConsecutiveFails = value; }
void Synchronize::setDisplayDistributions(bool value) { _displayDistributions = value; }

Synchronize::Synchronize(Mechanics::Device* refDevice,
                         Mechanics::Device* dutDevice,
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
  _syncRatioSample(100),
  _maxOffset(100),
  _maxLargeSyncAttempts(20),
  _bufferSize(10),
  _preDiscards(2),
  _displayDistributions(false),
  _maxConsecutiveFails(3)
{
  assert(refInput && dutInput && refOutput && dutOutput &&
         "Looper: initialized with null object(s)");
}

}
