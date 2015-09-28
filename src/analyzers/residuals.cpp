#include "residuals.h"

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

void Residuals::processEvent(const Storage::Event* refEvent)
{
  assert(refEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent);

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

    for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
    {
      Storage::Plane* plane = refEvent->getPlane(nplane);
      Mechanics::Sensor* sensor = _device->getSensor(nplane);
      double tx = 0, ty = 0, tz = 0;
      Processors::trackSensorIntercept(track, sensor, tx, ty, tz);

      for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
      {
        Storage::Cluster* cluster = plane->getCluster(ncluster);

        // Check if the cluster passes the cuts
        for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
          if (!_clusterCuts.at(ncut)->check(cluster)) continue;

        const double rx = tx - cluster->getPosX();
        const double ry = ty - cluster->getPosY();
        const double rz = tz - cluster->getPosZ();	
        _residualsX.at(nplane)->Fill(rx);
        _residualsY.at(nplane)->Fill(ry);
        _residualsZ.at(nplane)->Fill(rz);

	// Compute the rotation difference
	/*
	double theta = atan2(ty,tx);
	double rr = cluster->getPosY()*sin(theta)+cluster->getPosY()*cos(theta);
	double distR = sqrt(tx*tx+ty*ty);
        _residualsRR.at(nplane)->Fill(distR, rr);
	*/
	_totResidual+=sqrt(rx*rx+ry*ry);
        _residualsXX.at(nplane)->Fill(rx, tx);
        _residualsYY.at(nplane)->Fill(ry, ty);
        _residualsXY.at(nplane)->Fill(rx, ty);
        _residualsYX.at(nplane)->Fill(ry, tx);
      }
    }
  }
}

void Residuals::postProcessing() { } // Needs to be declared even if not used

TH2D* Residuals::getResidualXX(unsigned int nsensor)
{
  validSensor(nsensor);
  return _residualsXX.at(nsensor);
}

TH2D* Residuals::getResidualXY(unsigned int nsensor)
{
  validSensor(nsensor);
  return _residualsXY.at(nsensor);
}

TH2D* Residuals::getResidualYY(unsigned int nsensor)
{
  validSensor(nsensor);
  return _residualsYY.at(nsensor);
}

TH2D* Residuals::getResidualYX(unsigned int nsensor)
{
  validSensor(nsensor);
  return _residualsYX.at(nsensor);
}

Residuals::Residuals(const Mechanics::Device* refDevice,
                     TDirectory* dir,
                     const char* suffix,
                     unsigned int nPixX,
                     double binsPerPix,
                     unsigned int binsY) :
  // Base class is initialized here and manages directory / device
  SingleAnalyzer(refDevice, dir, suffix),
  _totResidual(0.0)
{
  assert(refDevice && "Analyzer: can't initialize with null device");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo
  
  // Makes or gets a directory called from inside _dir with this name
  name.str("");
  name << "Residuals1D" << suffix;
  TDirectory* dir1d = makeGetDirectory(name.str().c_str());
  name.str("");
  name << "Residuals2D" << suffix;  
  TDirectory* dir2d = makeGetDirectory(name.str().c_str());

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _device->getSensor(nsens);
    
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
	 <<  "RvsR" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
	  << " R vs. R" 
	  << ";Track cluster difference R "
	  << " [" << _device->getSpaceUnit() << "]"
	  << ";Track position " << "R "
	  << " [" << _device->getSpaceUnit() << "]"
	  << ";Clusters / " << 1.0 / binsPerPix << " pixel";
    TH2D* resRR = new TH2D(name.str().c_str(), title.str().c_str(),
			   100, 0.0, 400.0,
			   100, -100.0, 100.0);
    resRR->SetDirectory(dir2d);
    _residualsRR.push_back(resRR);
    
    for (unsigned int axis = 0; axis < 3; axis++)
    {
// HP try
      const double width =  (axis==3) ? 30.0 : (axis==2) ? 30.0 : nPixX * 
          (axis ? sensor->getPosPitchX() : sensor->getPosPitchY());
      unsigned int nbins = 2.0 * binsPerPix * nPixX;
      if (!(nbins % 2)) nbins += 1;
      double height = 0;

      // Generate the 1D residual distribution for the given axis
      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis==3) ? "R" : (axis==2) ? "Z" : (axis) ? "X" : "Y") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis==3) ? "R" : (axis==2) ? "Z" : (axis) ? " X" : " Y")
            << ";Track cluster difference " << ((axis==3) ? "R" : (axis==2) ? "Z" : axis ? "X" : "Y")
            << " [" << _device->getSpaceUnit() << "]"
            << ";Clusters / " << 1.0 / binsPerPix << " pixel";
      TH1D* res1d = new TH1D(name.str().c_str(), title.str().c_str(),
                             nbins, -width / 2.0, width / 2.0);
      res1d->SetDirectory(dir1d);
      if (axis==1)      _residualsX.push_back(res1d);
      else if(axis==0)  _residualsY.push_back(res1d);
      else if(axis==2)  _residualsZ.push_back(res1d);
      //else if(axis==3)  _residualsR.push_back(res1d);      

      // Generate the XX or YY residual distribution
      if(axis==2 || axis==3) continue; // Z is not programmed, so we skip it
      
      // The height of this plot depends on the sensor and X or Y axis
      height =  axis ? sensor->getPosSensitiveX() : sensor->getPosSensitiveY(); //(axis==2) ? sensor->getPosSensitiveZ() :

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis) ? "XvsX" : "YvsY") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis) ? " X vs. X" : " Y vs. Y")
            << ";Track cluster difference " << (axis ? "X" : "Y")
            << " [" << _device->getSpaceUnit() << "]"
            << ";Track position " << (axis ? "X" : "Y")
            << " [" << _device->getSpaceUnit() << "]"
            << ";Clusters / " << 1.0 / binsPerPix << " pixel";
      TH2D* resAA = new TH2D(name.str().c_str(), title.str().c_str(),
                             nbins, -width / 2.0, width / 2.0,
                             binsY, -height / 2.0, height / 2.0);
      resAA->SetDirectory(dir2d);
      if (axis) _residualsXX.push_back(resAA);
      else      _residualsYY.push_back(resAA);

      // Generate the XY or YX distribution
      height = axis ? sensor->getSensitiveY() : sensor->getSensitiveX();

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << ((axis) ? "XvsY" : "YvsX") << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << ((axis) ? " X vs. Y" : " Y vs. X")
            << ";Track cluster difference " << (axis ? "X" : "Y")
            << " [" << _device->getSpaceUnit() << "]"
            << ";Track position " << (axis ? "Y" : "X")
            << " [" << _device->getSpaceUnit() << "]"
            << ";Clusters / " << 1.0 / binsPerPix << " pixel";
      TH2D* resAB = new TH2D(name.str().c_str(), title.str().c_str(),
                             nbins, -width / 2.0, width / 2.0,
                             binsY, -height / 2.0, height / 2.0);
      resAB->SetDirectory(dir2d);
      if (axis) _residualsXY.push_back(resAB);
      else      _residualsYX.push_back(resAB);
    }
  }
}

}
