#include "chi2align.h"

#include <cassert>
#include <vector>
#include <list>
#include <utility>
#include <cmath>
#include <iostream>
#include <sstream>

#include <Rtypes.h>
#include <Math/Minimizer.h>
#include <Math/Factory.h>

#include "../storage/storageio.h"
#include "../storage/event.h"
#include "../storage/cluster.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../mechanics/alignment.h"
#include "../processors/processors.h"
#include "../processors/clustermaker.h"
#include "../processors/trackmaker.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"
#include "../analyzers/cuts.h"
#include "../analyzers/residuals.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

namespace Loopers {

double Chi2Align::Chi2Minimizer::DoEval(const double* pars) const {
  double sum = 0;

  double rotation[2][2] = { 0 };
  const double norm = _tracklets.size();

  // Evaluate the sum of Chi2
  for (std::vector<Tracklet>::const_iterator it = _tracklets.begin();
      it != _tracklets.end(); ++it) {
    const Tracklet& tracklet = *it;

    const double rx0 = tracklet.n0 > 0 ? pars[(tracklet.n0-1)*NPARS+ROTX] : 0;
    const double ry0 = tracklet.n0 > 0 ? pars[(tracklet.n0-1)*NPARS+ROTY] : 0;
    const double rz0 = tracklet.n0 > 0 ? pars[(tracklet.n0-1)*NPARS+ROTZ] : 0;
    const double tx0 = tracklet.n0 > 0 ? pars[(tracklet.n0-1)*NPARS+TRANSX] : 0;
    const double ty0 = tracklet.n0 > 0 ? pars[(tracklet.n0-1)*NPARS+TRANSY] : 0;

    rotation[0][0] = cos(ry0) * cos(rz0);
    rotation[0][1] = -cos(rx0) * sin(rz0) + sin(rx0) * sin(ry0) * cos(rz0);
    rotation[1][0] = cos(ry0) * sin(rz0);
    rotation[1][1] = cos(rx0) * cos(rz0) + sin(rx0) * sin(ry0) * sin(rz0);

    const double x0 = tx0 +
        tracklet.x0 * rotation[0][0] +
        tracklet.y0 * rotation[0][1];
    const double y0 = ty0 +
        tracklet.x0 * rotation[1][0] +
        tracklet.y0 * rotation[1][1];
    const double ex0 = 
        tracklet.ex0 * rotation[0][0] +
        tracklet.ey0 * rotation[0][1];
    const double ey0 = 
        tracklet.ex0 * rotation[1][0] +
        tracklet.ey0 * rotation[1][1];

    const double rx1 = tracklet.n1 > 0 ? pars[(tracklet.n1-1)*NPARS+ROTX] : 0;
    const double ry1 = tracklet.n1 > 0 ? pars[(tracklet.n1-1)*NPARS+ROTY] : 0;
    const double rz1 = tracklet.n1 > 0 ? pars[(tracklet.n1-1)*NPARS+ROTZ] : 0;
    const double tx1 = tracklet.n1 > 0 ? pars[(tracklet.n1-1)*NPARS+TRANSX] : 0;
    const double ty1 = tracklet.n1 > 0 ? pars[(tracklet.n1-1)*NPARS+TRANSY] : 0;

    rotation[0][0] = cos(ry1) * cos(rz1);
    rotation[0][1] = -cos(rx1) * sin(rz1) + sin(rx1) * sin(ry1) * cos(rz1);
    rotation[1][0] = cos(ry1) * sin(rz1);
    rotation[1][1] = cos(rx1) * cos(rz1) + sin(rx1) * sin(ry1) * sin(rz1);

    const double x1 = tx1 +
        tracklet.x1 * rotation[0][0] +
        tracklet.y1 * rotation[0][1];
    const double y1 = ty1 +
        tracklet.x1 * rotation[1][0] +
        tracklet.y1 * rotation[1][1];
    const double ex1 = 
        tracklet.ex1 * rotation[0][0] +
        tracklet.ey1 * rotation[0][1];
    const double ey1 = 
        tracklet.ex1 * rotation[1][0] +
        tracklet.ey1 * rotation[1][1];

    const double errX = ex0*ex0 + ex1*ex1;
    const double errY = ey0*ey0 + ey1*ey1;
    const double chi2 = std::pow(x0-x1, 2)/errX + std::pow(y0-y1, 2)/errY;

    sum += chi2/norm;
  }
  
  std::cout << sum << std::endl;
  return sum;
}

ROOT::Math::IBaseFunctionMultiDim* Chi2Align::Chi2Minimizer::Clone() const {
  return new Chi2Align::Chi2Minimizer(_device, _tracklets);
}

void Chi2Align::loop()
{
  std::list<Tracklet> tracklets;

  double sumchi2 = 0;

  // Build the tracklets which will be used in the computation
  for (ULong64_t nevent = _startEvent; nevent <= _endEvent; nevent++)
  {
    Storage::Event* refEvent = _refStorage->readEvent(nevent);

    if (refEvent->getNumClusters())
      throw "Chi2Align: can't recluster an event, mask the tree in the input";

    // Cluster the event
    for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes(); nplane++)
      _clusterMaker->generateClusters(refEvent, nplane);

    // Apply the alignment to the event 
    Processors::applyAlignment(refEvent, _refDevice);

    for (unsigned int nplane = 0; nplane < refEvent->getNumPlanes()-1; nplane++)
    {
      Storage::Plane& plane0 = *refEvent->getPlane(nplane);
      Storage::Plane& plane1 = *refEvent->getPlane(nplane+1);
      Mechanics::Sensor& sensor0 = *_refDevice->getSensor(nplane);
      Mechanics::Sensor& sensor1 = *_refDevice->getSensor(nplane+1);

      for (unsigned int ncluster0 = 0; ncluster0 < plane0.getNumClusters(); ncluster0++)
      {
        Storage::Cluster& c0 = *plane0.getCluster(ncluster0);

        for (unsigned int ncluster1 = 0; ncluster1 < plane1.getNumClusters(); ncluster1++)
        {
          Storage::Cluster& c1 = *plane1.getCluster(ncluster1);

          const double errX = 
              std::pow(c0.getPosErrX(), 2) +
              std::pow(c1.getPosErrX(), 2);
          const double errY =
              std::pow(c0.getPosErrY(), 2) +
              std::pow(c1.getPosErrY(), 2);
          const double chi2 =
              std::pow(c0.getPosX()-c1.getPosX(), 2) / errX +
              std::pow(c0.getPosY()-c1.getPosY(), 2) / errY;

          if (chi2 > _maxChi2) continue;

          sumchi2 += chi2;

          // Build the tracklet: the segment connecting the two clusters. Do
          // not use alignment for these values.

          tracklets.push_back(Tracklet());

          tracklets.back().n0 = nplane;
          tracklets.back().n1 = nplane+1;

          tracklets.back().x0 =
            c0.getPixX() * sensor0.getPitchX() - sensor0.getSensitiveX()/2.;
          tracklets.back().y0 =
            c0.getPixY() * sensor0.getPitchY() - sensor0.getSensitiveY()/2.;
          tracklets.back().z0 = sensor0.getOffZ();
          tracklets.back().ex0 = c0.getPixErrX() * sensor0.getPitchX();
          tracklets.back().ey0 = c0.getPixErrY() * sensor0.getPitchY();

          tracklets.back().x1 =
            c1.getPixX() * sensor1.getPitchX() - sensor1.getSensitiveX()/2.;
          tracklets.back().y1 =
            c1.getPixY() * sensor1.getPitchY() - sensor1.getSensitiveY()/2.;
          tracklets.back().z1 = sensor1.getOffZ();
          tracklets.back().ex1 = c1.getPixErrX() * sensor1.getPitchX();
          tracklets.back().ey1 = c1.getPixErrY() * sensor1.getPitchY();
        }
      }
    }

    progressBar(nevent);

    delete refEvent;
  }

  // Build the default minimizer (probably Minuit with Migrad)
  ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer();

  Chi2Minimizer evaluator(*_refDevice, tracklets);
  minimizer->SetFunction(evaluator);

  std::cout << "Initial chi2 " << sumchi2/(double)tracklets.size() << std::endl;

  // Set the variables to minimize (alignment parameters) and their initial
  // values
  std::stringstream ss;
  for (unsigned int nsensor = 1; nsensor < _refDevice->getNumSensors(); nsensor++)
  {
    const Mechanics::Sensor& sensor = *_refDevice->getSensor(nsensor);

    ss.str("");
    ss << "RotX" << nsensor;
    minimizer->SetVariable(
        // Variables come in sets of npars per plane, except first plane
        (nsensor-1)*NPARS+ROTX,
        ss.str().c_str(),
        // Initial value is the current sensor alignment
        sensor.getRotX(),
        // Guess 1% of full angular range as step size
        2*3.14159/100.);

    ss.str("");
    ss << "RotY" << nsensor;
    minimizer->SetVariable(
        (nsensor-1)*NPARS+ROTY,
        ss.str().c_str(),
        sensor.getRotY(),
        2*3.14159/100.);

    ss.str("");
    ss << "RotZ" << nsensor;
    minimizer->SetVariable(
        (nsensor-1)*NPARS+ROTZ,
        ss.str().c_str(),
        sensor.getRotZ(),
        2*3.14159/100.);

    // Compute the spatial distance between (0,0) and (1,1)
    double x0, y0, z0;
    sensor.pixelToSpace(0,0,x0,y0,z0);
    double x1, y1, z1;
    sensor.pixelToSpace(1,1,x1,y1,z1);
    const double stepTransX = std::fabs(x0-x1);
    const double stepTransY = std::fabs(y0-y1);

    ss.str("");
    ss << "TransX" << nsensor;
    minimizer->SetVariable(
        (nsensor-1)*NPARS+TRANSX,
        ss.str().c_str(),
        sensor.getOffX(),
        stepTransX);

    ss.str("");
    ss << "TransY" << nsensor;
    minimizer->SetVariable(
        (nsensor-1)*NPARS+TRANSY,
        ss.str().c_str(),
        sensor.getOffY(),
        stepTransY);
  }

  std::cout << "Minimizer dims: " << minimizer->NDim() << std::endl;
  std::cout << "Evaluator dims: " << evaluator.NDim() << std::endl;
  for (unsigned int i = 0; i < minimizer->NDim(); i++) {
    std::cout << "Dim " << i << std::flush;
    std::cout << ", " << minimizer->VariableName(i) << std::flush;
    std::cout << std::endl;
  }

  minimizer->Minimize();

  for (unsigned int nsensor = 1; nsensor < _refDevice->getNumSensors(); nsensor++)
  {
    Mechanics::Sensor& sensor = *_refDevice->getSensor(nsensor);
    sensor.setRotX(minimizer->X()[(nsensor-1)*NPARS+ROTX]);
    sensor.setRotY(minimizer->X()[(nsensor-1)*NPARS+ROTY]);
    sensor.setRotZ(minimizer->X()[(nsensor-1)*NPARS+ROTZ]);
    sensor.setOffX(minimizer->X()[(nsensor-1)*NPARS+TRANSX]);
    sensor.setOffY(minimizer->X()[(nsensor-1)*NPARS+TRANSY]);
  }

  _refDevice->getAlignment()->writeFile();

  delete minimizer;
}

void Chi2Align::setMaxChi2(double value) { _maxChi2 = value; }

Chi2Align::Chi2Align(Mechanics::Device* refDevice,
                     Processors::ClusterMaker* clusterMaker,
                     Storage::StorageIO* refInput,
                     ULong64_t startEvent,
                     ULong64_t numEvents,
                     Long64_t eventSkip) :
  Looper(refInput, 0, startEvent, numEvents, eventSkip),
  _refDevice(refDevice),
  _clusterMaker(clusterMaker),
  _maxChi2(10)
{
  assert(refInput && refDevice && clusterMaker && 
         "Looper: initialized with null object(s)");
  assert(refInput->getNumPlanes() == refDevice->getNumSensors() &&
         "Loopers: number of planes / sensors mis-match");
}

}
