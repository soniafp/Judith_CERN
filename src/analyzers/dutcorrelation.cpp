#include "dutcorrelation.h"

#include <cassert>
#include <sstream>
#include <math.h>

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TCanvas.h> //added Matevz 20141130

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

void DUTCorrelation::processEvent(const Storage::Event* refEvent,
                                  const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);

  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(refEvent)) return;

  for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
  {
    Storage::Plane* plane0 = refEvent->getPlane(_nearestRef);
    Storage::Plane* plane1 = dutEvent->getPlane(nsensor);

    TH2D* corrX = _corrX.at(nsensor);
    TH2D* corrY = _corrY.at(nsensor);
    TH1D* alignX = _alignX.at(nsensor);
    TH1D* alignY = _alignY.at(nsensor);

    //Matevz 20141202 Alignment histograms for pads
    TH1D* alignPadX = NULL;
    TH1D* alignPadY = NULL;
    if ( _dutDevice->getSensor(nsensor)->getNumX() == 1 )
      alignPadX = _alignPadX.at(nsensor);
    if ( _dutDevice->getSensor(nsensor)->getNumY() == 1 )
      alignPadY = _alignPadY.at(nsensor);


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

        //Matevz 20141202 Correlation and alignment histograms for pads
        //fill a histogram on refplane's position X/Y if there was a
        //hit in both replane's cluster AND dutplane's pad
        if ( _dutDevice->getSensor(nsensor)->getNumX() == 1 )
          alignPadX->Fill( cluster0->getPosX() );
        if ( _dutDevice->getSensor(nsensor)->getNumY() == 1 )
          alignPadY->Fill( cluster0->getPosY() );
      }
    }
  }
}

void DUTCorrelation::postProcessing() { } // Needs to be declared even if not used

void DUTCorrelation::initializeHist(const Mechanics::Sensor* sensor0,
                                    const Mechanics::Sensor* sensor1)
{
  std::stringstream name;
  std::stringstream title;

  std::stringstream axisTitleX;
  std::stringstream axisTitleY;
  std::stringstream axisTitle;

  for (unsigned int axis = 0; axis < 2; axis++)
  {
    axisTitle << "Clusters";

    // Get the number of pixels for the axis
    const unsigned int npix0 = (axis) ? sensor0->getPosNumX() : sensor0->getPosNumY();
    const unsigned int npix1 = (axis) ? sensor1->getPosNumX() : sensor1->getPosNumY();

    const double sens0Low = (axis) ? sensor0->getOffX() - sensor0->getPosSensitiveX() / 2.0 :
                                     sensor0->getOffY() - sensor0->getPosSensitiveY() / 2.0;
    const double sens0Upp = (axis) ? sensor0->getOffX() + sensor0->getPosSensitiveX() / 2.0 :
                                     sensor0->getOffY() + sensor0->getPosSensitiveY() / 2.0;

    const double sens1Low = (axis) ? sensor1->getOffX() - sensor1->getPosSensitiveX() / 2.0 :
                                     sensor1->getOffY() - sensor1->getPosSensitiveY() / 2.0;
    const double sens1Upp = (axis) ? sensor1->getOffX() + sensor1->getPosSensitiveX() / 2.0 :
                                     sensor1->getOffY() + sensor1->getPosSensitiveY() / 2.0;

    // Generate the name and title of the DUTCorrelation plot for this sensor and axis
    name.str(""); title.str("");
    name << sensor1->getDevice()->getName() << sensor1->getName()
         << "To" << sensor0->getDevice()->getName() << sensor0->getName()
         << ((axis) ? "X" : "Y") << _nameSuffix;
    title << sensor1->getDevice()->getName() << " " << sensor1->getName()
          << " To " << sensor0->getDevice()->getName() << " " << sensor0->getName()
          << ((axis) ? " X" : " Y");
    TH2D* corr = new TH2D(name.str().c_str(), title.str().c_str(),
                          npix0, sens0Low, sens0Upp,
                          npix1, sens1Low, sens1Upp);
    axisTitleX.str(""); axisTitleY.str("");
    axisTitleX << ((axis) ? "X " : "Y ") << " position on " << sensor0->getName()
               << " [" << _refDevice->getSpaceUnit() << "]";
    axisTitleY << ((axis) ? "X " : "Y ") << " position on " << sensor1->getName()
               << " [" << _refDevice->getSpaceUnit() << "]";

    corr->GetXaxis()->SetTitle(axisTitleX.str().c_str());
    corr->GetYaxis()->SetTitle(axisTitleY.str().c_str());
    corr->GetZaxis()->SetTitle(axisTitle.str().c_str());

    corr->SetDirectory(_corrDir);
    if (axis) _corrX.push_back(corr);
    else      _corrY.push_back(corr);

    const double sens1Size = (axis) ? sensor1->getPosSensitiveX() :
                                      sensor1->getPosSensitiveY();

    // Repeat for the alignment histograms
    name.str(""); title.str("");
    name << sensor1->getDevice()->getName() << sensor1->getName()
         << "To" << sensor0->getDevice()->getName() << sensor0->getName()
         << ((axis) ? "X" : "Y") << _nameSuffix;
    title << sensor1->getDevice()->getName() << " " << sensor1->getName()
          << " To " << sensor0->getDevice()->getName() << " " << sensor0->getName()
          << ((axis) ? " X" : " Y");
    TH1D* align = new TH1D(name.str().c_str(), title.str().c_str(),
			   2 * npix0 - 1, sens0Low, sens0Upp); // use the telescope instead of the DUT because there is only 1 pixel
                           //2 * npix1 - 1, -sens1Size, sens1Size);
    axisTitleX.str("");
    axisTitleX << ((axis) ? "X " : "Y ") << " position difference"
               << " [" << _refDevice->getSpaceUnit() << "]";

    align->GetXaxis()->SetTitle(axisTitleX.str().c_str());
    align->GetYaxis()->SetTitle(axisTitle.str().c_str());

    align->SetDirectory(_alignDir);
    if (axis) _alignX.push_back(align);
    else      _alignY.push_back(align);



    //Matevz 20141202 Pad alignment histograms - initialisation
    const double sens0Size = (axis) ? sensor0->getPosSensitiveX() :
                                      sensor0->getPosSensitiveY();
    name.str(""); title.str("");
    name << "Pad align "
         << sensor1->getDevice()->getName() << sensor1->getName()
         << "To" << sensor0->getDevice()->getName() << sensor0->getName()
         << ((axis) ? "X" : "Y") << _nameSuffix;
    title << "Pad align "
          << sensor1->getDevice()->getName() << " " << sensor1->getName()
          << " To " << sensor0->getDevice()->getName() << " " << sensor0->getName()
          << ((axis) ? " X" : " Y");
    TH1D* alignPad = new TH1D(name.str().c_str(), title.str().c_str(),
                           2 * npix0 - 1, -sens0Size, sens0Size);
    axisTitleX.str("");
    axisTitleX << ((axis) ? "X " : "Y ") << " position difference"
               << " [" << _refDevice->getSpaceUnit() << "]";

    alignPad->GetXaxis()->SetTitle(axisTitleX.str().c_str());
    alignPad->GetYaxis()->SetTitle(axisTitle.str().c_str());

    alignPad->SetDirectory(_alignDir);
    if (axis) _alignPadX.push_back(alignPad);
    else      _alignPadY.push_back(alignPad);


  }
}

//Matevz 20141202 Getters for pad alignment plots
TH1D* DUTCorrelation::getPadAlignmentPlotX(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _alignPadX.at(nsensor);
}

TH1D* DUTCorrelation::getPadAlignmentPlotY(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _alignPadY.at(nsensor);
}




TH1D* DUTCorrelation::getAlignmentPlotX(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _alignX.at(nsensor);
}

TH1D* DUTCorrelation::getAlignmentPlotY(unsigned int nsensor)
{
  validDutSensor(nsensor);
  return _alignY.at(nsensor);
}

DUTCorrelation::DUTCorrelation(const Mechanics::Device* refDevice,
                               const Mechanics::Device* dutDevice,
                               TDirectory* dir,
                               const char* suffix) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  _alignDir = makeGetDirectory("DUTAlignment");
  _corrDir = makeGetDirectory("DUTCorrelations");

  assert(_dutDevice->getNumSensors() > 0 && "DUTCorrelations: device should have planes");

  Mechanics::Sensor* firstDut = _dutDevice->getSensor(0);
  Mechanics::Sensor* nearest = 0;

  // Determine which is the nearest ref plane
  for (unsigned int nsens = 0; nsens < _refDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* compareRef = _refDevice->getSensor(nsens);
    if (!nearest || fabs(compareRef->getOffZ() - firstDut->getOffZ()) <
        fabs(nearest->getOffZ() - firstDut->getOffZ()))
    {
      nearest = compareRef;
      _nearestRef = nsens;
    }
  }
  std::cout << "DUTCorrelation::DUTCorrelation - nearest: " << nearest
	    << " nearestRef: " << _nearestRef << std::endl;

  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
    initializeHist(nearest, _dutDevice->getSensor(nsens));
}

}
