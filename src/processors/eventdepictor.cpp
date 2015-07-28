#include "eventdepictor.h"

#include <cassert>
#include <sstream>
#include <vector>

#include <TH2D.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TLine.h>
#include <TEllipse.h>
#include <TBox.h>
#include <TGraphErrors.h>
#include <TROOT.h>
#include <TColor.h>
#include <TStyle.h>

#include "../storage/event.h"
#include "../storage/track.h"
#include "../storage/plane.h"
#include "../storage/cluster.h"
#include "../storage/hit.h"
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
#include "../processors/processors.h"

namespace Processors {

void EventDepictor::drawEventTrackIntercepts(const Storage::Track* track,
                                             const Storage::Cluster* linkedCluster,
                                             const Mechanics::Sensor* sensor)
{
  double posX = 0, posY = 0, posZ = 0;
  const int code = Processors::trackSensorIntercept(track, sensor, posX, posY, posZ);
  if (code) return; // Track doesn't intercept this sensor (it's parallel)

  double errX = 0, errY = 0;
  Processors::trackError(track, posZ, errX, errY);

  // If a cluster is passed, depcit the track offset from this cluster by the zoom
  const double offX = linkedCluster ? linkedCluster->getPosX() : 0;
  const double offY = linkedCluster ? linkedCluster->getPosY() : 0;

  const double x1 = linkedCluster ?
                    offX + _zoom * (posX - offX) - _zoom * errX :
                    posX - _zoom * errX;
  const double x0 = linkedCluster ?
                    offX + _zoom * (posX - offX) :
                    posX;
  const double x2 = linkedCluster ?
                    offX + _zoom * (posX - offX) + _zoom * errX :
                    posX + _zoom * errX;

  const double y1 = linkedCluster ?
                    offY + _zoom * (posY - offY) - _zoom * errY :
                    posY - _zoom * errY;
  const double y0 = linkedCluster ?
                    offY + _zoom * (posY - offY) :
                    posY;
  const double y2 = linkedCluster ?
                    offY + _zoom * (posY - offY) + _zoom * errY :
                    posY + _zoom * errY;

  double px1 = 0, py1 = 0;
  sensor->spaceToPixel(x1, y1, posZ, px1, py1);

  double px0 = 0, py0 = 0;
  sensor->spaceToPixel(x0, y0, posZ, px0, py0);

  double px2 = 0, py2 = 0;
  sensor->spaceToPixel(x2, y2, posZ, px2, py2);

  TLine* lx = new TLine(px1, py0, px2, py0);
  TLine* ly = new TLine(px0, py1, px0, py2);

  // Draw a different color if it has an associated cluster
  lx->SetLineColor(linkedCluster ? 9 : 46);
  lx->SetLineWidth(1);
  lx->Draw();

  ly->SetLineColor(linkedCluster ? 9 : 46);
  ly->SetLineWidth(1);
  ly->Draw();
}

void EventDepictor::drawEventClusterHits(const Storage::Cluster* cluster)
{
  for (unsigned int nhit = 0; nhit < cluster->getNumHits(); nhit++)
  {
    Storage::Hit* hit = cluster->getHit(nhit);

    assert(hit->getCluster() == cluster && "EventDepictor: hit / cluster association failed");

    const double hitX = cluster->getPixX() + _zoom * (hit->getPixX() + 0.5 - cluster->getPixX());
    const double hitY = cluster->getPixY() + _zoom * (hit->getPixY() + 0.5 - cluster->getPixY());
    const double hitX1 = hitX - _zoom / 2.0;
    const double hitX2 = hitX + _zoom / 2.0;
    const double hitY1 = hitY - _zoom / 2.0;
    const double hitY2 = hitY + _zoom / 2.0;

    TBox* box = new TBox(hitX1, hitY1, hitX2, hitY2);
    box->SetLineStyle(1);
    box->SetLineColor(23);
    box->SetLineWidth(1);
    box->SetFillStyle(1);
    box->SetFillColor(19);
    box->Draw("l");
  }
}

void EventDepictor::drawEventCluster(const Storage::Cluster* cluster)
{
  TEllipse* el = new TEllipse(cluster->getPixX(), cluster->getPixY(),
                              _zoom * cluster->getPixErrX(),
                              _zoom * cluster->getPixErrY());
  el->SetFillStyle(0);
  el->SetLineColor(28);
  el->SetLineWidth(1);
  el->Draw();
}

void EventDepictor::depictEventSensor(const Storage::Plane* plane,
                                 const Mechanics::Sensor* sensor)
{
  std::stringstream name;
  std::stringstream title;
  std::cout << "    DEVICE_NAME: "<< sensor->getDevice()->getName() << std::endl
       << "    sensor->getName():" << sensor->getName() <<std::endl;
       
  name << sensor->getDevice()->getName() << sensor->getName() << "Depiction";
  title << sensor->getDevice()->getName() << " " << sensor->getName() << "Depiction";

  TH2D* depiction = new TH2D(name.str().c_str(), title.str().c_str(),
                             sensor->getNumX(), 0 - 0.5, sensor->getNumX() + 0.5,
                             sensor->getNumY(), 0 - 0.5, sensor->getNumY() + 0.5);
  depiction->SetStats(0);
  depiction->SetDirectory(0);

  // Some error checking
  for (unsigned int nhit = 0; nhit < plane->getNumHits(); nhit++)
    assert(plane->getHit(nhit)->getCluster() &&
           "Processors: unclustered hit detected during depiction");

  depiction->Draw(); // Draw axis and sets coordinates for drawing boxes...

  for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
  {
    Storage::Cluster* cluster = plane->getCluster(ncluster);
    drawEventClusterHits(cluster);
    drawEventCluster(cluster);
  }
}

void EventDepictor::depictClusters(std::vector<const Storage::Cluster*>& refClusters,
                                   std::vector<const Storage::Cluster*>& dutClusters)
{
  // Check if the canvas has already been drawn, and try to retrieve it
  TCanvas* can = 0;
  can = (TCanvas*)gDirectory->FindObject("DepicClusterCanvas");
  if (!can) can = new TCanvas("DepicClusterCanvas", "Depict Cluster Canvas", 900, 900);

  std::stringstream title;

  title << "Cluster Depiction;"
        << "X Axis [" << _refDevice->getSpaceUnit() << "];"
        << "Y Axis [" << _refDevice->getSpaceUnit() << "];";

  const double sizeFactor = 1.5;
  Mechanics::Sensor* refSensor = _refDevice->getSensor(0);
  const unsigned int nbinsX = sizeFactor * refSensor->getNumX();
  const unsigned int nbinsY = sizeFactor * refSensor->getNumY();
  const double halfX = sizeFactor * refSensor->getSensitiveX() / 2.0;
  const double halfY = sizeFactor * refSensor->getSensitiveY() / 2.0;

  TH2D* depiction = new TH2D("ClusterDepiction", title.str().c_str(),
                             nbinsX, -halfX, halfX,
                             nbinsY, -halfY, halfY);
  depiction->SetStats(0);
  depiction->SetDirectory(0);

  depiction->Draw(); // Draw axis and sets coordinates for drawing boxes...

  for (unsigned int ncluster = 0; ncluster < refClusters.size(); ncluster++)
  {
    const Storage::Cluster* cluster = refClusters.at(ncluster);

    TEllipse* el = new TEllipse(cluster->getPosX(), cluster->getPosY(),
                                _zoom * cluster->getPosErrX(),
                                _zoom * cluster->getPosErrY());

    el->SetFillStyle(3002);
    el->SetFillColor(9);
    el->SetLineColor(9);
    el->SetLineWidth(1);
    el->Draw();
  }

  for (unsigned int ncluster = 0; ncluster < dutClusters.size(); ncluster++)
  {
    const Storage::Cluster* cluster = dutClusters.at(ncluster);

    TEllipse* el = new TEllipse(cluster->getPosX(), cluster->getPosY(),
                                _zoom * cluster->getPosErrX(),
                                _zoom * cluster->getPosErrY());

    el->SetFillStyle(3002);
    el->SetFillColor(46);
    el->SetLineColor(46);
    el->SetLineWidth(1);
    el->Draw();
  }

  can->WaitPrimitive();
}

void EventDepictor::depictTrack(const Storage::Track* track)
{
  assert(track && "EventDepictor: can't depict with null track");

  for (unsigned int ncluster = 0; ncluster < track->getNumClusters(); ncluster++)
    assert(track->getCluster(ncluster)->getTrack() == track &&
           "DepictTrack: cluster track didn't associated");
  for (unsigned int ncluster = 0; ncluster < track->getNumMatchedClusters(); ncluster++)
    assert(track->getMatchedCluster(ncluster)->getMatchedTrack() == track &&
           "DepictTrack: matched cluster track didn't associated");

  const char* name = "DepictTrack";
  const char* title = "Track Depiction";

  // Check if the canvas has already been drawn, and try to retrieve it
  TCanvas* can = 0;
  can = (TCanvas*)gDirectory->FindObject(name);
  if (!can) can = new TCanvas(name, title, 1000, 800);
  can->Divide(1, 2);

  double* px = new double[track->getNumClusters() + track->getNumMatchedClusters()];
  double* ex = new double[track->getNumClusters() + track->getNumMatchedClusters()];
  double* py = new double[track->getNumClusters() + track->getNumMatchedClusters()];
  double* ey = new double[track->getNumClusters() + track->getNumMatchedClusters()];
  double* pz = new double[track->getNumClusters() + track->getNumMatchedClusters()];

  for (unsigned int ncluster = 0; ncluster < track->getNumClusters(); ncluster++)
  {
    Storage::Cluster* cluster = track->getCluster(ncluster);
    px[ncluster] = cluster->getPosX();
    ex[ncluster] = cluster->getPosErrX();
    py[ncluster] = cluster->getPosY();
    ey[ncluster] = cluster->getPosErrY();
    pz[ncluster] = cluster->getPosZ();
  }

  for (unsigned int ncluster = 0; ncluster < track->getNumMatchedClusters(); ncluster++)
  {
    Storage::Cluster* cluster = track->getMatchedCluster(ncluster);
    px[track->getNumClusters() + ncluster] = cluster->getPosX();
    ex[track->getNumClusters() + ncluster] = cluster->getPosErrX();
    py[track->getNumClusters() + ncluster] = cluster->getPosY();
    ey[track->getNumClusters() + ncluster] = cluster->getPosErrY();
    pz[track->getNumClusters() + ncluster] = cluster->getPosZ();
  }

  double minZ = _refDevice->getSensor(0)->getOffZ();
  double maxZ = _refDevice->getSensor(_refDevice->getNumSensors() - 1)->getOffZ();

  if (_dutDevice && _dutDevice->getSensor(0)->getOffZ() < minZ)
    minZ = _dutDevice->getSensor(0)->getOffZ();
  if (_dutDevice && _dutDevice->getSensor(_dutDevice->getNumSensors() - 1)->getOffZ() > maxZ)
    maxZ = _dutDevice->getSensor(_dutDevice->getNumSensors() - 1)->getOffZ();

  for (unsigned int axis = 0; axis < 2; axis++)
  {
    std::stringstream ss;
    ss << "Slope: " << (axis ? track->getSlopeX() : track->getSlopeY())
       << " Chi^{2}: " << track->getChi2();

    TGraphErrors* graph = new TGraphErrors(track->getNumClusters() + track->getNumMatchedClusters(),
                                           pz, axis ? px : py, 0, axis ? ex : ey);
    graph->SetTitle(ss.str().c_str());
    graph->GetXaxis()->SetTitle("Z position");
    graph->GetYaxis()->SetTitle(axis ? "X position" : "Y position");

    const double par0 = axis ? track->getOriginX() : track->getOriginY();
    const double par1 = axis ? track->getSlopeX() : track->getSlopeY();
    const double par2 = pow(axis ? track->getOriginErrX() : track->getOriginErrY(), 2);
    const double par3 = pow(axis ? track->getSlopeErrX() : track->getSlopeErrY(), 2);
    const double par4 = axis ? track->getCovarianceX() : track->getCovarianceY();

    TF1* errUpp = new TF1(axis ? "TrackUpperErrorX" : "TrackUpperErrorY",
                          "[0] + [1]*x + sqrt([2] + [3]*x^2 + [4]*x)", minZ, maxZ);
    errUpp->SetParameter(0, par0);
    errUpp->SetParameter(1, par1);
    errUpp->SetParameter(2, par2);
    errUpp->SetParameter(3, par3);
    errUpp->SetParameter(4, par4);
    errUpp->SetLineColor(46);
    errUpp->SetLineWidth(1);

    TF1* errLow = new TF1(axis ? "TrackLowerErrorX" : "TrackLowerErrorX",
                          "[0] + [1]*x - sqrt([2] + [3]*x^2 + [4]*x)", minZ, maxZ);
    errLow->SetParameter(0, par0);
    errLow->SetParameter(1, par1);
    errLow->SetParameter(2, par2);
    errLow->SetParameter(3, par3);
    errLow->SetParameter(4, par4);
    errLow->SetLineColor(46);
    errLow->SetLineWidth(1);

    can->cd(axis + 1);
    errUpp->Draw();
    graph->Draw("AP");
    errUpp->Draw("SAME");
    errLow->Draw("SAME");
  }

  can->WaitPrimitive();

  delete[] px;
  delete[] ex;
  delete[] py;
  delete[] ey;
  delete[] pz;
}

void EventDepictor::depictEvent(const Storage::Event* refEvent,
                                const Storage::Event* dutEvent)
{
  assert((refEvent && refEvent->getNumPlanes() == _refDevice->getNumSensors()) &&
         "EventDepictor: device event plane mis-match");
  assert((!dutEvent || (_dutDevice && dutEvent->getNumPlanes() == _dutDevice->getNumSensors())) &&
         "EventDepictor: device event plane mis-match or no dut device");

  if (_zoom < 1) throw "EventDepictor: can't deipct with a zoom < 1";

  const unsigned int cols = 3;
  const unsigned int numSensors = refEvent->getNumPlanes() +
      (dutEvent ? dutEvent->getNumPlanes() : 0);
  const unsigned int rows = numSensors / cols + ((numSensors % cols) ? 1 : 0);

  const unsigned int width = cols * 300;
  const unsigned int height = rows * 300;

  const char* name = "DepictEventPlanes";
  const char* title = "Event Plane Depiction";

  // Check if the canvas has already been drawn, and try to retrieve it
  TCanvas* can = 0;
  can = (TCanvas*)gDirectory->FindObject(name);
  if (!can) can = new TCanvas(name, title, width, height);

  can->Divide(cols, rows);

  for (unsigned int nsens = 0; nsens < _refDevice->getNumSensors(); nsens++)
  {
    can->cd(nsens + 1);
    const Storage::Plane* eventPlane = refEvent->getPlane(nsens);
    depictEventSensor(eventPlane, _refDevice->getSensor(nsens));
  }

  // Now for the DUT events if a DUT is present
  if (dutEvent)
  {
    for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
    {
      can->cd(nsens + _refDevice->getNumSensors() + 1);
      const Storage::Plane* eventPlane = dutEvent->getPlane(nsens);
      depictEventSensor(eventPlane, _dutDevice->getSensor(nsens));
    }
  }

  // Add track intercepts
  for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
  {
    const Storage::Track* track = refEvent->getTrack(ntrack);

    // Generate arrays of the clusters associated with this track
    Storage::Cluster** refClusters = new Storage::Cluster*[_refDevice->getNumSensors()];
    for (unsigned int nsens = 0; nsens < _refDevice->getNumSensors(); nsens++)
      refClusters[nsens] = 0;

    for (unsigned int nclus = 0; nclus < track->getNumClusters(); nclus++)
    {
      Storage::Cluster* cluster = track->getCluster(nclus);
      refClusters[cluster->getPlane()->getPlaneNum()] = cluster;
    }

    // Draw the track in each sensor, with the associated cluster
    for (unsigned int nsens = 0; nsens < _refDevice->getNumSensors(); nsens++)
    {
      Storage::Cluster* cluster = refClusters[nsens];
      Mechanics::Sensor* sensor = _refDevice->getSensor(nsens);
      can->cd(nsens + 1);
      drawEventTrackIntercepts(track, cluster, sensor);
    }

    delete[] refClusters;

    // And for the DUT but don't associate with clusters
    if (_dutDevice)
    {
      for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
      {
        Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);
        can->cd(nsens + _refDevice->getNumSensors() + 1);
        drawEventTrackIntercepts(track, 0, sensor);
      }
    }
  }

  can->WaitPrimitive();
}


EventDepictor::EventDepictor(const Mechanics::Device* refDevice,
                             const Mechanics::Device* dutDevice) :
  _zoom(20),
  _refDevice(refDevice),
  _dutDevice(dutDevice)
{
  assert(refDevice && "EventDepictor: can't initialize with null ref device");
}

}
