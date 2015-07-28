#include "hitinfo.h"

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

void HitInfo::processEvent(const Storage::Event* event)
{
  assert(event && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(event);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(event)) return;

  for (unsigned int nplane = 0; nplane < event->getNumPlanes(); nplane++)
  {
    Storage::Plane* plane = event->getPlane(nplane);

    for (unsigned int nhit = 0; nhit < plane->getNumHits(); nhit++)
    {
      Storage::Hit* hit = plane->getHit(nhit);

      // Check if the hit passes the cuts
      bool pass = true;
      for (unsigned int ncut = 0; ncut < _numHitCuts; ncut++)
        if (!_hitCuts.at(ncut)->check(hit)) { pass = false; break; }
      if (!pass) continue;


      _lvl1.at(nplane)->Fill(hit->getTiming());
      _tot.at(nplane)->Fill(hit->getValue());
      _totMap.at(nplane)->Fill(hit->getPixX(), hit->getPixY(), hit->getValue());
      _totMapCnt.at(nplane)->Fill(hit->getPixX(), hit->getPixY());
    }
  }
}

void HitInfo::postProcessing()
{
  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    TH2D* map = _totMap.at(nsens);
    TH2D* count = _totMapCnt.at(nsens);

    for (Int_t x = 1; x <= map->GetNbinsX(); x++)
    {
      for (Int_t y = 1; y <= map->GetNbinsY(); y++)
      {
        const double average = map->GetBinContent(x, y) / count->GetBinContent(x, y);
        map->SetBinContent(x, y, average);
      }
    }
  }
}

HitInfo::HitInfo(const Mechanics::Device* device,
                 TDirectory* dir,
                 const char* suffix,
                 unsigned int lvl1Bins,
                 unsigned int totBins) :
  // Base class is initialized here and manages directory / device
  SingleAnalyzer(device, dir, suffix),
  _lvl1Bins(lvl1Bins),
  _totBins(totBins)
{
  assert(device && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("HitInfo");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _device->getSensor(nsens);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "Timing" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Level 1 Accept"
          << ";Level 1 bin number"
          << ";Hits";
    TH1D* lvl1 = new TH1D(name.str().c_str(), title.str().c_str(),
                          _lvl1Bins, 0 - 0.5, _lvl1Bins - 0.5);
    lvl1->SetDirectory(plotDir);
    _lvl1.push_back(lvl1);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "ToT" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " ToT Distribution"
          << ";ToT bin number"
          << ";Hits";
    TH1D* tot = new TH1D(name.str().c_str(), title.str().c_str(),
                         _totBins, 0 - 0.5, _totBins - 0.5);
    tot->SetDirectory(plotDir);
    _tot.push_back(tot);

    name.str(""); title.str("");
    name << sensor->getName() << "ToTMap" << _nameSuffix;
    title << sensor->getName() << " ToT Map"
          << ";X pixel"
          << ";Y pixel"
          << ";Average ToT";
    TH2D* totMap = new TH2D(name.str().c_str(), title.str().c_str(),
                            sensor->getNumX(), 0 - 0.5, sensor->getNumX() - 0.5,
                            sensor->getNumY(), 0 - 0.5, sensor->getNumY() - 0.5);
    totMap->SetDirectory(plotDir);
    _totMap.push_back(totMap);

    name.str(""); title.str("");
    name << sensor->getName() << "ToTMapCnt" << _nameSuffix;
    title << sensor->getName() << " ToT Map Counter";
    TH2D* totMapCnt = new TH2D(name.str().c_str(), title.str().c_str(),
                               sensor->getNumX(), 0 - 0.5, sensor->getNumX() - 0.5,
                               sensor->getNumY(), 0 - 0.5, sensor->getNumY() - 0.5);
    totMapCnt->SetDirectory(0);
    _totMapCnt.push_back(totMapCnt);
  }
}

}
