#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <algorithm>
#include <cmath>

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

      // Get the bounding box of the sensor

      // Get the x, y coordinates of two opposite sensor corners
      double sensX1 = 0;
      double sensX2 = 0;
      double sensY1 = 0;
      double sensY2 = 0;
      sensor.getSensorBox(sensX1, sensY1, sensX2, sensY2);

      // Get the sensor resolution with rotations
      double sensResX = 0;
      double sensResY = 0;
      sensor.getPixBox(sensResX, sensResY);

      for (size_t iref = 0; iref < refDevice.getNumSensors(); iref++) {
        const Mechanics::Sensor& ref = refDevice[iref];

        // Repetitive, but this is a one time initialization so its ok I guess
        double refX1 = 0;
        double refX2 = 0;
        double refY1 = 0;
        double refY2 = 0;
        ref.getSensorBox(refX1, refY1, refX2, refY2);

        double refResX = 0;
        double refResY = 0;
        ref.getPixBox(refResX, refResY);

        // Spatial extent of the sensor (largest of two scenarios: either the
        // sensor is shifted to the left, or the reference is)
        const double sizeX = std::max(sensX2-refX1, refX2-sensX2);
        const double sizeY = std::max(sensY2-refY1, refY2-sensY2);

        // Don't bin more finely than the distance between two pixel which
        // barely overlap but see the same particle (should prevent aliasing
        // features in the plots, better for fitting this way)
        const double resX = sensResX/2. + refResX/2.;
        const double resY = sensResY/2. + refResY/2.;

        const std::string name = 
            device.m_name + "_" + sensor.m_name + "_" + ref.m_name;

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

