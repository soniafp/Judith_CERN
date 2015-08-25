#include "correlation.h"

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

void Correlation::processEvent(const Storage::Event* event)
{
  assert(event && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(event);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(event)) return;

  for (unsigned int nsens = 0; nsens < _device->getNumSensors() - 1; nsens++)
  {
    Storage::Plane* plane0 = event->getPlane(nsens);
    Storage::Plane* plane1 = event->getPlane(nsens + 1);

    TH2D* corrX = _corrX.at(nsens);
    TH2D* corrY = _corrY.at(nsens);
    TH1D* alignX = _alignX.at(nsens);
    TH1D* alignY = _alignY.at(nsens);

    for (unsigned int ncluster0 = 0; ncluster0 < plane0->getNumClusters(); ncluster0++)
    {
      Storage::Cluster* cluster0 = plane0->getCluster(ncluster0);

      // Check if the cluster passes the cuts
      bool pass = true;
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster0)) { pass = false; break; }
      if (!pass) continue;

      for (unsigned int ncluster1 = 0; ncluster1 < plane1->getNumClusters(); ncluster1++)
      {
        Storage::Cluster* cluster1 = plane1->getCluster(ncluster1);

        // Check if the cluster passes the cuts
        bool pass = true;
        for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
          if (!_clusterCuts.at(ncut)->check(cluster1)) { pass = false; break; }
        if (!pass) continue;

        corrX->Fill(cluster0->getPosX(), cluster1->getPosX());
        corrY->Fill(cluster0->getPosY(), cluster1->getPosY());
        alignX->Fill(cluster1->getPosX() - cluster0->getPosX());
        alignY->Fill(cluster1->getPosY() - cluster0->getPosY());
      }
    }
  }
}

void Correlation::postProcessing() { } // Needs to be declared even if not used

void Correlation::initializeHist(const Mechanics::Sensor* sensor0,
                                 const Mechanics::Sensor* sensor1)
{
  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  std::stringstream axisTitleX;
  std::stringstream axisTitleY;
  std::stringstream axisTitle;

  axisTitle << "Clusters";

  for (unsigned int axis = 0; axis < 2; axis++)
  {
    // Generate the name and title of the correlation plot for this sensor and axis
    name.str(""); title.str("");
    name << sensor1->getDevice()->getName() << sensor1->getName()
         << "To" << sensor0->getDevice()->getName() << sensor0->getName()
         << ((axis) ? "X" : "Y") << _nameSuffix;
    title << sensor1->getDevice()->getName() << " " << sensor1->getName()
          << " To " << sensor0->getDevice()->getName() << " " << sensor0->getName()
          << ((axis) ? " X" : " Y");

    // Get the number of pixels for the axis
    const unsigned int npix0 = (axis) ? sensor0->getNumX() : sensor0->getPosNumY();
    const unsigned int npix1 = (axis) ? sensor1->getNumX() : sensor1->getPosNumY();

    const double sens0Low = (axis) ? sensor0->getOffX() - sensor0->getPosSensitiveX() / 2.0 :
                                     sensor0->getOffY() - sensor0->getPosSensitiveY() / 2.0;
    const double sens0Upp = (axis) ? sensor0->getOffX() + sensor0->getPosSensitiveX() / 2.0 :
                                     sensor0->getOffY() + sensor0->getPosSensitiveY() / 2.0;

    const double sens1Low = (axis) ? sensor1->getOffX() - sensor1->getPosSensitiveX() / 2.0 :
                                     sensor1->getOffY() - sensor1->getPosSensitiveY() / 2.0;
    const double sens1Upp = (axis) ? sensor1->getOffX() + sensor1->getPosSensitiveX() / 2.0 :
                                     sensor1->getOffY() + sensor1->getPosSensitiveY() / 2.0;

    // Generate the histogram and label its axis
    TH2D* corr = new TH2D(name.str().c_str(), title.str().c_str(),
                          npix0, sens0Low, sens0Upp,
                          npix1, sens1Low, sens1Upp);

    axisTitleX.str(""); axisTitleY.str("");
    axisTitleX << ((axis) ? "X " : "Y ") << " position on " << sensor0->getName()
               << " [" << _device->getSpaceUnit() << "]";
    axisTitleY << ((axis) ? "X " : "Y ") << " position on " << sensor1->getName()
               << " [" << _device->getSpaceUnit() << "]";

    corr->GetXaxis()->SetTitle(axisTitleX.str().c_str());
    corr->GetYaxis()->SetTitle(axisTitleY.str().c_str());
    corr->GetZaxis()->SetTitle(axisTitle.str().c_str());

    // Add to the vector of histograms for future access during filling
    corr->SetDirectory(_corrDir);
    if (axis) _corrX.push_back(corr);
    else      _corrY.push_back(corr);

    // Repeat for the alignment histograms
    name.str(""); title.str("");
    name << sensor1->getDevice()->getName() << sensor1->getName()
         << "To" << sensor0->getDevice()->getName() << sensor0->getName()
         << ((axis) ? "X" : "Y") << _nameSuffix;
    title << sensor1->getDevice()->getName() << " " << sensor1->getName()
          << " To " << sensor0->getDevice()->getName() << " " << sensor0->getName()
          << ((axis) ? " X" : " Y");

    const double sens1Size = (axis) ? sensor1->getPosSensitiveX() :
                                      sensor1->getPosSensitiveY();

    // Aliasing issue if you don't subtract from npix
    TH1D* align = new TH1D(name.str().c_str(), title.str().c_str(),
                           2*(2 * npix1 - 1), -sens1Size, sens1Size);

    axisTitleX.str("");
    axisTitleX << ((axis) ? "X " : "Y ") << " position difference"
               << " [" << _device->getSpaceUnit() << "]";

    align->GetXaxis()->SetTitle(axisTitleX.str().c_str());
    align->GetYaxis()->SetTitle(axisTitle.str().c_str());

    align->SetDirectory(_alignDir);
    if (axis) _alignX.push_back(align);
    else      _alignY.push_back(align);
  }
}

TH1D* Correlation::getAlignmentPlotX(unsigned int nsensor)
{
  if (nsensor == 0) throw "Correlation: no plot exists for sensor 0";
  nsensor -= 1;
  validSensor(nsensor);
  return _alignX.at(nsensor);
}

TH1D* Correlation::getAlignmentPlotY(unsigned int nsensor)
{
  if (nsensor == 0) throw "Correlation: no plot exists for sensor 0";
  nsensor -= 1;
  validSensor(nsensor);
  return _alignY.at(nsensor);
}

Correlation::Correlation(const Mechanics::Device* device,
                         TDirectory* dir,
                         const char* suffix) :
  // Base class is initialized here and manages directory / device
  SingleAnalyzer(device, dir, suffix)
{
  assert(device && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  _corrDir = makeGetDirectory("Correlations");
  _alignDir = makeGetDirectory("Alignment");

  for (unsigned int nsens = 0; nsens < _device->getNumSensors() - 1; nsens++)
    initializeHist(_device->getSensor(nsens), _device->getSensor(nsens + 1));
}

}
