#include "exampledualanalyzer.h"

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

namespace Analyzers {

void ExampleDualAnalyzer::processEvent(const Storage::Event* refEvent,
                                       const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(refEvent)) return;

  // Loop over tracks, clusters, hits... and apply cuts to each object above
}

void ExampleDualAnalyzer::postProcessing() { } // Needs to be declared even if not used

ExampleDualAnalyzer::ExampleDualAnalyzer(const Mechanics::Device* refDevice,
                                         const Mechanics::Device* dutDevice,
                                         TDirectory* dir,
                                         const char* suffix,
                                         int dummyParameter,
                                         unsigned int histBins) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix),
  // Initialize processing parameters here
  _dummyParameter(dummyParameter)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("ExampleDual");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);
    name.str(""); title.str("");
    name << _dutDevice->getName() << sensor->getName() << "DummySensorHist" << _nameSuffix;
    title << _dutDevice->getName() << " " << sensor->getName() << " Dummy Sensor Hist";
    TH1D* hist = new TH1D(name.str().c_str(), title.str().c_str(),
                          histBins, 0, 100);
    hist->GetXaxis()->SetTitle("Axis Title");
    hist->SetDirectory(plotDir);
    _dummySensorHist.push_back(hist);
  }

  name.str(""); title.str("");
  name << _dutDevice->getName() << "DummyHist" << _nameSuffix;
  title << _dutDevice->getName() << " Dummy Hist";
  _dummyHist = new TH2D(name.str().c_str(), title.str().c_str(),
                       histBins, 0, 100,
                       histBins, 0, 100);
  _dummyHist->GetXaxis()->SetTitle("Axis Title");
  _dummyHist->GetYaxis()->SetTitle("Axis Title");
  _dummyHist->SetDirectory(plotDir);
}

}
