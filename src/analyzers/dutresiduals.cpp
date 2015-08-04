#include "dutresiduals.h"

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

void DUTResiduals::processEvent(const Storage::Event* refEvent,
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

    for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
    {
      Storage::Plane* plane = dutEvent->getPlane(nplane);
      Mechanics::Sensor* sensor = _dutDevice->getSensor(nplane);
      double tx = 0, ty = 0, tz = 0;
      Processors::trackSensorIntercept(track, sensor, tx, ty, tz);

      for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
      {
        Storage::Cluster* cluster = plane->getCluster(ncluster);

        // Check if the cluster passes the cuts
        bool pass = true;
        for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
          if (!_clusterCuts.at(ncut)->check(cluster)) { pass = false; break; }
        if (!pass) continue;

        const double rx = tx - cluster->getPosX();
        const double ry = ty - cluster->getPosY();
        _residualsX.at(nplane)->Fill(rx);
        _residualsY.at(nplane)->Fill(ry);
        _residualsXX.at(nplane)->Fill(rx, tx);
        _residualsYY.at(nplane)->Fill(ry, ty);
        _residualsXY.at(nplane)->Fill(rx, ty);
        _residualsYX.at(nplane)->Fill(ry, tx);
      }
    }
  }
}

void DUTResiduals::postProcessing() { } // Needs to be declared even if not used

TH2D* DUTResiduals::getResidualXX(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _residualsXX.at(nsensor);
}

TH2D* DUTResiduals::getResidualXY(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _residualsXY.at(nsensor);
}

TH2D* DUTResiduals::getResidualYY(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _residualsYY.at(nsensor);
}

TH2D* DUTResiduals::getResidualYX(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _residualsYX.at(nsensor);
}

DUTResiduals::DUTResiduals(const Mechanics::Device* refDevice,
                           const Mechanics::Device* dutDevice,
                           TDirectory* dir,
                           const char* suffix,
                           unsigned int nPixX,
                           double binsPerPix,
                           unsigned int binsY) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* dir1d = makeGetDirectory("DUTResiduals1D");
  TDirectory* dir2d = makeGetDirectory("DUTResiduals2D");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  std::stringstream xAxisTitle;
  std::stringstream yAxisTitle;

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);
    for (unsigned int axis = 0; axis < 2; axis++)
    {
      const double width = nPixX * (axis ? sensor->getPosPitchX() : sensor->getPosPitchY());
      unsigned int nbins = binsPerPix * nPixX;
      if (!(nbins % 2)) nbins += 1;
      double height = 0;

      xAxisTitle.str("");
      xAxisTitle << "Track cluster difference " << (axis ? "X" : "Y")
                 << " [" << _refDevice->getSpaceUnit() << "]";

      // Generate the 1D residual distribution for the given axis
      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis) ? "X" : "Y") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis) ? " X" : " Y");
      TH1D* res1d = new TH1D(name.str().c_str(), title.str().c_str(),
                             nbins*4, -width * 2.0, width * 2.0);
      res1d->SetDirectory(dir1d);
      res1d->GetXaxis()->SetTitle(xAxisTitle.str().c_str());
      if (axis) _residualsX.push_back(res1d);
      else      _residualsY.push_back(res1d);

      // Generate the XX or YY residual distribution

      // The height of this plot depends on the sensor and X or Y axis
      height = axis ? sensor->getSensitiveX() : sensor->getSensitiveY();
      yAxisTitle.str("");
      yAxisTitle << "Track position " << (axis ? "X" : "Y")
                 << " [" << _refDevice->getSpaceUnit() << "]";

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis) ? "XvsX" : "YvsY") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis) ? " X vs. X" : " Y vs. Y");
      TH2D* resAA = new TH2D(name.str().c_str(), title.str().c_str(),
                             nbins, -width / 2.0, width / 2.0,
                             binsY, -height / 2.0, height / 2.0);
      resAA->SetDirectory(dir2d);
      resAA->GetXaxis()->SetTitle(xAxisTitle.str().c_str());
      resAA->GetYaxis()->SetTitle(yAxisTitle.str().c_str());
      if (axis) _residualsXX.push_back(resAA);
      else      _residualsYY.push_back(resAA);

      // Generate the XY or YX distribution

      height = axis ? sensor->getSensitiveY() : sensor->getSensitiveX();
      yAxisTitle.str("");
      yAxisTitle << "Track position " << (axis ? "Y" : "X")
                 << " [" << _refDevice->getSpaceUnit() << "]";

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis) ? "XvsY" : "YvsX") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis) ? " X vs. Y" : " Y vs. X");
      TH2D* resAB = new TH2D(name.str().c_str(), title.str().c_str(),
                             nbins, -width / 2.0, width / 2.0,
                             binsY, -height / 2.0, height / 2.0);
      resAB->SetDirectory(dir2d);
      resAB->GetXaxis()->SetTitle(xAxisTitle.str().c_str());
      resAB->GetYaxis()->SetTitle(yAxisTitle.str().c_str());
      if (axis) _residualsXY.push_back(resAB);
      else      _residualsYX.push_back(resAB);
    }
  }
}

}
