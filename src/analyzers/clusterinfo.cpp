#include "clusterinfo.h"

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

void ClusterInfo::processEvent(const Storage::Event* event)
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

    for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
    {
      Storage::Cluster* cluster = plane->getCluster(ncluster);

      // Check if the cluster passes the cuts
      bool pass = true;
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster)) { pass = false; break; }
      if (!pass) continue;

      _clusterSize.at(nplane)->Fill(cluster->getNumHits());
      _tot.at(nplane)->Fill(cluster->getValue());
      _totSize.at(nplane)->Fill(cluster->getNumHits(), cluster->getValue());

      if (_clustersVsTime.size())
        _clustersVsTime.at(nplane)->Fill(_device->tsToTime(event->getTimeStamp()));
      if (_totVsTime.size())
        _totVsTime.at(nplane)->Fill(_device->tsToTime(event->getTimeStamp()), cluster->getValue());
    }
  }
}

void ClusterInfo::postProcessing()
{
  if (_postProcessed) return;

  if (_clustersVsTime.size())
  {
    for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
    {
      TH1D* hist = _totVsTime.at(nsens);
      TH1D* cnt  = _clustersVsTime.at(nsens);
      for (Int_t bin = 1; bin <= hist->GetNbinsX(); bin++)
      {
        if (cnt->GetBinContent(bin) < 1) continue;
        hist->SetBinContent(bin, hist->GetBinContent(bin) / (double)cnt->GetBinContent(bin));
      }
    }
  }

  _postProcessed = true;
}

ClusterInfo::ClusterInfo(const Mechanics::Device* device,
                         TDirectory* dir,
                         const char* suffix,
                         unsigned int totBins,
                         unsigned int maxClusterSize) :
  // Base class is initialized here and manages directory / device
  SingleAnalyzer(device, dir, suffix),
  _totBins(totBins)
{
  assert(device && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("ClusterInfo");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _device->getSensor(nsens);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "Size" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Cluster Size"
          << ";Pixels in cluster"
          << ";Clusters";
    TH1D* size = new TH1D(name.str().c_str(), title.str().c_str(),
                          maxClusterSize - 1, 1 - 0.5, maxClusterSize - 0.5);
    size->SetDirectory(plotDir);
    _clusterSize.push_back(size);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "ToT" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Clustered ToT Distribution"
          << ";ToT bin number"
          << ";Clusters";
    TH1D* tot = new TH1D(name.str().c_str(), title.str().c_str(),
                         _totBins, 0 - 0.5, _totBins - 0.5);
    tot->SetDirectory(plotDir);
    _tot.push_back(tot);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "ToTVsSize" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " ToT Vs. Cluster Size"
          << ";Pixels in cluster"
          << ";ToT bin number"
          << ";Clusters";
    TH2D* totSize = new TH2D(name.str().c_str(), title.str().c_str(),
                             maxClusterSize - 1, 1 - 0.5, maxClusterSize - 0.5,
                             _totBins, 0 - 0.5, _totBins - 0.5);
    totSize->SetDirectory(plotDir);
    _totSize.push_back(totSize);

    if (_device->getTimeEnd() > _device->getTimeStart()) // If not used, they are both == 0
    {
      // Prevent aliasing
      const unsigned int nTimeBins = 100;
      const ULong64_t timeSpan = _device->getTimeEnd() - _device->getTimeStart() + 1;
      const ULong64_t startTime = _device->getTimeStart();
      const ULong64_t endTime = timeSpan - (timeSpan % nTimeBins) + startTime;

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << "ClustersVsTime" << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << " Clustsers Vs. Time";
      TH1D* clusterTime = new TH1D(name.str().c_str(), title.str().c_str(),
                                   nTimeBins,
                                   _device->tsToTime(startTime),
                                   _device->tsToTime(endTime + 1));
      clusterTime->SetDirectory(0);
      _clustersVsTime.push_back(clusterTime);

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << "TotVsTime" << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << " ToT Vs. Time"
            << ";Time [" << _device->getTimeUnit() << "]"
            << ";Average cluster ToT";
      TH1D* totTime = new TH1D(name.str().c_str(), title.str().c_str(),
                               nTimeBins,
                               _device->tsToTime(startTime),
                               _device->tsToTime(endTime + 1));
      totTime->SetDirectory(plotDir);
      _totVsTime.push_back(totTime);
    }
  }
}

}
