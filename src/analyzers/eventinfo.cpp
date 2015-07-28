#include "eventinfo.h"

#include <cassert>
#include <sstream>
#include <math.h>

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
// This header defines all the cuts
#include "cuts.h"

#include <iostream>

namespace Analyzers {

void EventInfo::processEvent(const Storage::Event* event)
{
  assert(event && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(event);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(event)) return;

  _triggerOffset->Fill(event->getTriggerOffset());
  _trackInTime->Fill(event->getTriggerOffset());
  _numTracks->Fill(event->getNumTracks());

  if (_eventsVsTime)
  {
    _eventsVsTime->Fill(_device->tsToTime(event->getTimeStamp()));
    _tracksVsTime->Fill(_device->tsToTime(event->getTimeStamp()), event->getNumTracks());
    _clustersVsTime->Fill(_device->tsToTime(event->getTimeStamp()), event->getNumClusters());
  }
}

void EventInfo::postProcessing()
{
  if (_postProcessed) return;

  if (_eventsVsTime)
  {
    for (Int_t bin = 1; bin <= _eventsVsTime->GetNbinsX(); bin++)
    {
      const double norm = _eventsVsTime->GetBinContent(bin);
      if (norm < 1) continue;
      _tracksVsTime->SetBinContent(bin, _tracksVsTime->GetBinContent(bin) / norm);
      _clustersVsTime->SetBinContent(bin, _clustersVsTime->GetBinContent(bin) / norm);
    }
  }

  _postProcessed = true;
}

EventInfo::EventInfo(const Mechanics::Device* device,
                     TDirectory* dir,
                     const char* suffix,
                     unsigned int maxTracks) :
  // Base class is initialized here and manages directory / device
  SingleAnalyzer(device, dir, suffix),
  _eventsVsTime(0),
  _tracksVsTime(0),
  _clustersVsTime(0)
{
  assert(device && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("EventInfo");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  const unsigned int nTrigBins = 100;
  const unsigned int lastTrigBin = _device->getReadOutWindow() -
      (_device->getReadOutWindow() % nTrigBins);

  name.str(""); title.str("");
  name << _device->getName() << "TriggerOffset" << _nameSuffix;
  title << _device->getName() << " Trigger Offset"
        << ";Trigger offset [clock cycles]"
        << ";Events / " << lastTrigBin / nTrigBins << " clock cycles";
  _triggerOffset = new TH1D(name.str().c_str(), title.str().c_str(),
                            nTrigBins, 0 - 0.5, lastTrigBin - 0.5);
  _triggerOffset->SetDirectory(plotDir);

  name.str(""); title.str("");
  name << _device->getName() << "TracksInTime" << _nameSuffix;
  title << _device->getName() << " Tracks In Time"
        << ";Trigger offset [clock cycles]"
        << ";Tracks / " << lastTrigBin / nTrigBins << " clock cycles";
  _trackInTime = new TH1D(name.str().c_str(), title.str().c_str(),
                          nTrigBins, 0 - 0.5, lastTrigBin - 0.5);
  _trackInTime->SetDirectory(plotDir);

  name.str(""); title.str("");
  name << _device->getName() << "Tracks" << _nameSuffix;
  title << _device->getName() << " Tracks Multiplicity"
        << ";Track multiplicity"
        << ";Tracks";
  _numTracks = new TH1D(name.str().c_str(), title.str().c_str(),
                        maxTracks + 1, 0  - 0.5, maxTracks + 1 - 0.5);
  _numTracks->SetDirectory(plotDir);

  if (_device->getTimeEnd() > _device->getTimeStart()) // If not used, they are both == 0
  {
    // Prevent aliasing
    const unsigned int nTimeBins = 100;
    const ULong64_t timeSpan = _device->getTimeEnd() - _device->getTimeStart() + 1;
    const ULong64_t startTime = _device->getTimeStart();
    const ULong64_t endTime = timeSpan - (timeSpan % nTimeBins) + startTime;

    name.str(""); title.str("");
    name << _device->getName() << "EventsVsTime" << _nameSuffix;
    title << _device->getName() << " Events Vs. Time"
          << ";Time [" << _device->getTimeUnit() << "]"
          << ";Events / " << timeSpan / (double)_device->getClockRate() / 100
          << " " << _device->getTimeUnit();
    _eventsVsTime = new TH1D(name.str().c_str(), title.str().c_str(),
                             nTimeBins,
                             _device->tsToTime(startTime),
                             _device->tsToTime(endTime + 1));
    _eventsVsTime->SetDirectory(plotDir);

    name.str(""); title.str("");
    name << _device->getName() << "TracksVsTime" << _nameSuffix;
    title << _device->getName() << " Tracks Vs. Time"
          << ";Time [" << _device->getTimeUnit() << "]"
          << ";Average tracks per event";
    _tracksVsTime = new TH1D(name.str().c_str(), title.str().c_str(),
                             nTimeBins,
                             _device->tsToTime(startTime),
                             _device->tsToTime(endTime + 1));
    _tracksVsTime->SetDirectory(plotDir);

    name.str(""); title.str("");
    name << _device->getName() << "ClustersVsTime" << _nameSuffix;
    title << _device->getName() << " Clusters Vs. Time"
          << ";Time [" << _device->getTimeUnit() << "]"
          << ";Average clusters per event";
    _clustersVsTime = new TH1D(name.str().c_str(), title.str().c_str(),
                               nTimeBins,
                               _device->tsToTime(startTime),
                               _device->tsToTime(endTime + 1));
    _clustersVsTime->SetDirectory(plotDir);
  }
}

}
