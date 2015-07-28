#include "syncfluctuation.h"

#include <cassert>
#include <sstream>
#include <math.h>
#include <iostream>
#include <algorithm>

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>
#include <Rtypes.h>

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
// This header defines all the cuts
#include "cuts.h"

using std::cout;
using std::endl;

namespace Analyzers {

void SyncFluctuation::processEvent(const Storage::Event* refEvent,
                                   const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(refEvent)) return;

  if (_counter > 0) // Want to have a lastRef and lastDut
  {
    const ULong64_t refClockDiff = (refEvent->getTimeStamp() - _lastRef);
    const double refFrameDiff = refClockDiff /
        (double)_refDevice->getReadOutWindow();

    const ULong64_t dutClockDiff = (dutEvent->getTimeStamp() - _lastDut);
    const double dutFrameDiff = dutClockDiff * _dutDevice->getSyncRatio() /
        (double)_refDevice->getReadOutWindow();

    if (_counter > 1)
    {
      // The synchronized change difference
      const double sync = ((refFrameDiff - dutFrameDiff) /
                           (refFrameDiff + dutFrameDiff)/2.);
      // Generate a fake unsynchronized change difference
      const double unsync = ((_counter % 2) ? refFrameDiff : _lastChangeRef) -
                            ((_counter % 2) ? _lastChangeDut : dutFrameDiff);

      _synchronized->Fill(sync);
      _unsynchronized->Fill(unsync);
    }

    _lastChangeRef = refFrameDiff;
    _lastChangeDut = dutFrameDiff;
  }

  _lastRef = refEvent->getTimeStamp();
  _lastDut = dutEvent->getTimeStamp();

  _counter++;
}

void SyncFluctuation::postProcessing() { }

TH1D* SyncFluctuation::getSynchronized()
{
  return _synchronized;
}

TH1D* SyncFluctuation::getUnsynchronized()
{
  return _unsynchronized;
}

SyncFluctuation::SyncFluctuation(const Mechanics::Device* refDevice,
                                 const Mechanics::Device* dutDevice,
                                 TDirectory* dir,
                                 const char* suffix,
                                 double syncRange,
                                 double unsyncRange) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix),
  _synchronized(0),
  _unsynchronized(0),
  _lastRef(0),
  _lastDut(0),
  _lastChangeRef(0),
  _lastChangeDut(0),
  _counter(0)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("SyncFluctuation");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  name.str(""); title.str("");
  name << _dutDevice->getName() << "SyncFlucutation" << _nameSuffix;
  title << _dutDevice->getName() << " Syncrhonized Fluctuations";
  _synchronized = new TH1D(name.str().c_str(), title.str().c_str(),
                           100, -syncRange, syncRange);
  _synchronized->GetXaxis()->SetTitle("Discrimating variable");
  _synchronized->GetYaxis()->SetTitle("Events");
  _synchronized->SetDirectory(plotDir);

  name.str(""); title.str("");
  name << _dutDevice->getName() << "UnsyncFlucutation" << _nameSuffix;
  title << _dutDevice->getName() << " Unsyncrhonized Fluctuations";
  _unsynchronized = new TH1D(name.str().c_str(), title.str().c_str(),
                             // Odd number of bins to have one centered at 0
                             31, -unsyncRange, unsyncRange);
  _unsynchronized->GetXaxis()->SetTitle("Discrimating variable");
  _unsynchronized->GetYaxis()->SetTitle("Events");
  _unsynchronized->SetDirectory(plotDir);
}

}
