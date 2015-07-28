#include "matching.h"

#include <cassert>
#include <sstream>
#include <math.h>
#include <iomanip>

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TF1.h>

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
#include <iomanip>

namespace Analyzers {

void Matching::processEvent(const Storage::Event* refEvent,
                            const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(refEvent)) return;

  for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
  {
    Storage::Track* track = refEvent->getTrack(ntrack);

    // Check if the track passes the cuts
    bool pass = true;
    for (unsigned int ncut = 0; ncut < _numTrackCuts; ncut++)
      if (!_trackCuts.at(ncut)->check(track)) { pass = false; break; }
    if (!pass) continue;

    // Make a list of the planes with their matched cluster
    std::vector<Storage::Cluster*> matches;
    for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
      matches.push_back(0); // No matches

    // Get the matches from the track
    for (unsigned int nmatch = 0; nmatch < track->getNumMatchedClusters(); nmatch++)
    {
      Storage::Cluster* cluster = track->getMatchedCluster(nmatch);

      // Check if this cluster passes the cuts
      bool pass = true;
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster)) { pass = false; break; }
      if (!pass) continue;

      matches.at(cluster->getPlane()->getPlaneNum()) = cluster;
    }

    for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
    {
      Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);
      Storage::Cluster* match = matches.at(nsens);

      if (!match) continue;

      _matchDist.at(nsens)->Fill(match->getMatchDistance());

      double tx = 0, ty = 0, tz = 0;
      Processors::trackSensorIntercept(track, sensor, tx, ty, tz);

      const double distX = tx - match->getPosX();
      const double distY = ty - match->getPosY();

      _matchDistX.at(nsens)->Fill(distX);
      _matchDistY.at(nsens)->Fill(distY);

      double px = 0, py = 0;
      sensor->spaceToPixel(tx, ty, tz, px, py);

      // Find the distance of this track to all pixels in its matched cluster
      for (unsigned int nhit = 0; nhit < match->getNumHits(); nhit++)
      {
        const Storage::Hit* hit = match->getHit(nhit);
        const double hitX = hit->getPixX() + 0.5;
        const double hitY = hit->getPixY() + 0.5;

        const double pixDistX = sensor->getPitchX() * (px - hitX);
        const double pixDistY = sensor->getPitchY() * (py - hitY);

        _inPixelTracks.at(nsens)->Fill(pixDistX, pixDistY);
        _inPixelTot.at(nsens)->Fill(pixDistX, pixDistY, hit->getValue());
      }
    }
  }
}

void Matching::postProcessing()
{
  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);

    TH2D* tot = _inPixelTot.at(nsens);
    TH2D* hits = _inPixelTracks.at(nsens);

    assert(hits->GetNbinsX() == tot->GetNbinsX() &&
           hits->GetNbinsY() == tot->GetNbinsY() &&
           "Matching: histograms should have the same number of bins");

    for (int nx = 1; nx <= tot->GetNbinsX(); nx++)
    {
      for (int ny = 1; ny <= tot->GetNbinsY(); ny++)
      {
        const unsigned int num = hits->GetBinContent(nx, ny);
        if (num == 0) continue;
        const double average = tot->GetBinContent(nx, ny) / (double)num;
        tot->SetBinContent(nx, ny, average);
      }
    }

    for (unsigned int axis = 0; axis < 2; axis++)
    {
      TH1D* dist = axis ? _matchDistX.at(nsens) :
                          _matchDistY.at(nsens);

      double pixelWidth = axis ? sensor->getPitchX() :
                                 sensor->getPitchY();
      double beamSigma = axis ? _refDevice->getSensor(0)->getPitchX() :
                                _refDevice->getSensor(0)->getPitchY();

      TF1* fit = Processors::fitPixelBeam(dist, pixelWidth, beamSigma, false);

      std::stringstream ss;
      ss << _dutDevice->getName() << sensor->getName() << "PixelBeamFit"
         << ( axis ? "X" : "Y") << _nameSuffix;
      _plotDir->WriteObject(fit, ss.str().c_str());
    }
  }
}

Matching::Matching(const Mechanics::Device* refDevice,
                   const Mechanics::Device* dutDevice,
                   TDirectory* dir,
                   const char* suffix,
                   double pixelExtension,
                   double maxMatchingDist,
                   double sigmaBins,
                   unsigned int pixBinsX,
                   unsigned int pixBinsY) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix),
  _plotDir(0)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  _plotDir = makeGetDirectory("Matching");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  unsigned int numBins = maxMatchingDist * sigmaBins;
  if (!numBins) numBins = 1;

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "Distance" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Matched Cluster Distance"
          << ";Radial distance [standard deviation]"
          << ";Tracks / " << std::setprecision(2) << maxMatchingDist / (double)numBins << " standard deviation";
    TH1D* matchDist = new TH1D(name.str().c_str(), title.str().c_str(),
                               numBins, 0, maxMatchingDist);
    matchDist->SetDirectory(_plotDir);
    _matchDist.push_back(matchDist);

    const double maxDistX = maxMatchingDist *
                            sqrt( pow(sensor->getPitchX() / sqrt(12.0), 2) +
                                  pow(_refDevice->getSensor(0)->getPitchX() / sqrt(12.0), 2));
    const double maxDistY = maxMatchingDist *
                            sqrt( pow(sensor->getPitchY() / sqrt(12.0), 2) +
                                  pow(_refDevice->getSensor(0)->getPitchY() / sqrt(12.0), 2));

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "DistanceX" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Matched Cluster Distance"
          << ";Distance in X [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks / " << std::setprecision(2) << 2 * maxDistX / (2 * numBins) << " " << _dutDevice->getSpaceUnit();
    TH1D* matchDistX = new TH1D(name.str().c_str(), title.str().c_str(),
                                2 * numBins, -maxDistX, maxDistX);
    matchDistX->SetDirectory(_plotDir);
    _matchDistX.push_back(matchDistX);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "DistanceY" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Matched Cluster Distance"
          << ";Distance in Y [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks / " << std::setprecision(2) << 2 * maxDistY / (2 * numBins) << " " << _dutDevice->getSpaceUnit();
    TH1D* matchDistY = new TH1D(name.str().c_str(), title.str().c_str(),
                                2 * numBins, -maxDistY, maxDistY);
    matchDistY->SetDirectory(_plotDir);
    _matchDistY.push_back(matchDistY);

    const unsigned int binsX = pixBinsX * pixelExtension;
    const unsigned int binsY = pixBinsY * pixelExtension;

    const double pixHalfWidth = sensor->getPitchX() / 2.0 * pixelExtension;
    const double pixHalfHeight = sensor->getPitchY() / 2.0 * pixelExtension;

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "InPixelTracks" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " In Pixel Track Occupancy"
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* inPixTracks = new TH2D(name.str().c_str(), title.str().c_str(),
                                 binsX, -pixHalfWidth, pixHalfWidth,
                                 binsY, -pixHalfHeight, pixHalfHeight);
    inPixTracks->SetDirectory(_plotDir);
    _inPixelTracks.push_back(inPixTracks);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "InPixelTot" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " In Pixel Average Cluster ToT"
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Average Cluster ToT";
    TH2D* inPixTot = new TH2D(name.str().c_str(), title.str().c_str(),
                              binsX, -pixHalfWidth, pixHalfWidth,
                              binsY, -pixHalfHeight, pixHalfHeight);
    inPixTot->SetDirectory(_plotDir);
    _inPixelTot.push_back(inPixTot);
  }
}

}
