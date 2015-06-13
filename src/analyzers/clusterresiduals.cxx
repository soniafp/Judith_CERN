#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <algorithm>

#include <TDirectory.h>
#include <TH1D.h>
#include <TH2D.h>

#include "storage/event.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "mechanics/device.h"
#include "mechanics/sensor.h"
#include "analyzers/clusterresiduals.h"

namespace Analyzers {

void ClusterResiduals::initialize() {
  if (m_devices.empty()) return;

  const Mechanics::Device& refDevice = *m_devices[0];

  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    const Mechanics::Device& device = *m_devices[idevice];

    for (size_t isensor = 0; isensor < device.getNumSensors(); isensor++) {
      const Mechanics::Sensor& sensor = device[isensor];

      // Get the x, y coordinates of two opposite sensor corners
      double sensX1= 0;
      double sensX2 = 0;
      double sensY1 = 0;
      double sensY2 = 0;
      double dummy = 0;
      sensor.pixelToSpace(
          0, 0,
          sensX1, sensY1, dummy);
      sensor.pixelToSpace(
          sensor.m_ncols-1, sensor.m_nrows-1, 
          sensX2, sensY2, dummy);

      for (size_t iref = 0; iref < refDevice.getNumSensors(); iref++) {
        const Mechanics::Sensor& ref = refDevice[iref];

        // Repetitive, but this is a one time initialization so its ok I guess
        double refX1= 0;
        double refX2 = 0;
        double refY1 = 0;
        double refY2 = 0;
        ref.pixelToSpace(
            0, 0,
            refX1, refY1, dummy);
        ref.pixelToSpace(
            ref.m_ncols-1, ref.m_nrows-1, 
            refX2, refY2, dummy);

        const std::string name = 
            device.m_name + "_" + sensor.m_name + "_" + ref.m_name;

        const double sizeX = std::max(
            // Right most sensor edge to left most reference edge
            std::max(sensX1, sensX2) - std::min(refX1, refX2),
            // Right most reference edge to left most sensor edge
            std::max(refX1, refX2) - std::min(sensX1, sensX2));

        const double sizeY = std::max(
            std::max(sensY1, sensY2) - std::min(refY1, refY2),
            std::max(refY1, refY2) - std::min(sensY1, sensY2));

        const double resX = std::max(sensor.m_colPitch, ref.m_colPitch);
        const double resY = std::max(sensor.m_rowPitch, ref.m_rowPitch);

        const size_t nbinsX = 2 * sizeX / resX;
        const size_t nbinsY = 2 * sizeY / resY;

        m_hResidualsX.push_back(new TH1D(
            ("ResX_" + name).c_str(),
            ("ResX_" + name).c_str(),
            nbinsX, -sizeX, +sizeX));
        m_hResidualsX.back()->GetXaxis()->SetTitle(
            ("#Delta x ["+device.m_spaceUnit+"]").c_str());
        m_hResidualsX.back()->GetZaxis()->SetTitle("Triggers");
        m_hResidualsX.back()->Sumw2();
        m_histograms.push_back(m_hResidualsX.back());

        m_hResidualsY.push_back(new TH1D(
            ("ResY_" + name).c_str(),
            ("ResY_" + name).c_str(),
            nbinsY, -sizeY, +sizeY));
        m_hResidualsY.back()->GetXaxis()->SetTitle(
            ("#Delta y ["+device.m_spaceUnit+"]").c_str());
        m_hResidualsY.back()->GetZaxis()->SetTitle("Triggers");
        m_hResidualsY.back()->Sumw2();
        m_histograms.push_back(m_hResidualsY.back());

      }  // reference sensor loop
    }  // device sensor loop
  }  // device loop
}

void ClusterResiduals::process() {
  // Keep track of the reference event (with all reference planes)
  assert(m_ndevices > 0 && "ensures > 0 events");
  const Storage::Event& refEvent = *m_events[0];

  size_t iglobal = 0;

  for (size_t ievent = 0; ievent < m_events.size(); ievent++) {
    const Storage::Event& event = *m_events[ievent];

    for (size_t iplane = 0; iplane < event.getNumPlanes(); iplane++) {
      const Storage::Plane& plane = event.getPlane(iplane);

      for (size_t iref = 0; iref < refEvent.getNumPlanes(); iref++) {
        const Storage::Plane& ref = refEvent.getPlane(iref);

        // TODO: can swap iref <--> iplane for reference event

        for (size_t icluster = 0; icluster < plane.getNumClusters(); icluster++) {
          const Storage::Cluster& cluster = plane.getCluster(icluster);

          double bestDistance = 0;
          const Storage::Cluster* nearest = 0;

          for (size_t ircluster = 0; ircluster < ref.getNumClusters(); ircluster++) {
            const Storage::Cluster& refCluster = ref.getCluster(ircluster);

            const double distance = std::sqrt(
                std::pow(cluster.getPosX()-refCluster.getPosX(), 2) +
                std::pow(cluster.getPosY()-refCluster.getPosY(), 2));

            if (distance < bestDistance || ircluster == 0) {
              bestDistance = distance;
              nearest = &refCluster;
            }
          }

          if (!nearest) continue;
        
          m_hResidualsX[iglobal]->Fill(cluster.getPosX() - nearest->getPosX());
          m_hResidualsY[iglobal]->Fill(cluster.getPosY() - nearest->getPosY());
        }

        iglobal += 1;
      }
    }
  }
}

size_t ClusterResiduals::toGlobal(
    size_t idevice, 
    size_t isensor, 
    size_t iref) const {
  assert(!m_devices.empty());
  const size_t nref = m_devices[0]->getNumSensors();

  // Simple counting of sensor up to the correct device. Infrequently called,
  // so no need to cache or map indices.
  size_t iglobal = 0;
  for (size_t id = 0; id < idevice; id++)
    iglobal += m_devices[id]->getNumSensors() * nref;
  iglobal += isensor * nref + iref;

  return iglobal;
}

TH1D& ClusterResiduals::getResidualX(
    size_t idevice,
    size_t isensor,
    size_t iref) const {
  const size_t iglobal = toGlobal(idevice, isensor, iref);
  return *m_hResidualsX[iglobal];
}

TH1D& ClusterResiduals::getResidualY(
    size_t idevice, 
    size_t isensor,
    size_t iref) const {
  const size_t iglobal = toGlobal(idevice, isensor, iref);
  return *m_hResidualsY[iglobal];
}

}

