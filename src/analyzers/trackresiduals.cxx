#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <algorithm>
#include <cmath>

#include <TDirectory.h>
#include <TH1D.h>

#include "storage/plane.h"
#include "storage/cluster.h"
#include "storage/event.h"
#include "storage/track.h"
#include "mechanics/device.h"
#include "mechanics/sensor.h"
#include "analyzers/trackresiduals.h"

namespace Analyzers {

void TrackResiduals::initialize() {
  if (m_devices.empty()) return;

  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    const Mechanics::Device& device = *m_devices[idevice];

    for (size_t isensor = 0; isensor < device.getNumSensors(); isensor++) {
      const Mechanics::Sensor& sensor = device[isensor];

      const std::string name = device.m_name + "_" + sensor.m_name;

      double sizeX = 0;
      double sizeY = 0;

      sensor.getSensorBox(sizeX, sizeY);

      double resX = 0;
      double resY = 0;

      sensor.getPixBox(resX, resY);

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
    }  // device sensor loop
  }  // device loop
}

void TrackResiduals::process() {
  // Keep track of the reference event (with all reference planes)
  assert(m_ndevices > 0 && "ensures > 0 events");
  const Storage::Event& refEvent = *m_events[0];

  // Extrapolate each track to each plane to get residuals
  for (size_t itrack = 0; itrack < refEvent.getNumTracks(); itrack++) {
    const Storage::Track& track = refEvent.getTrack(itrack);

    // Start with residuals to clusters in the track (from the reference device)
    for (size_t icluster = 0; icluster < track.getNumClusters(); icluster++) {
      const Storage::Cluster& cluster = track.getCluster(icluster);

      // Get the plane index of this cluster. Note that this this is in the
      // reference device, the plane number is also its global number
      assert(cluster.fetchPlane() && "Can't have a cluster with no plane");
      const size_t iplane = cluster.fetchPlane()->getPlaneNum();

      // TODO: could use closest approach, but tracks should be in z direction
      // so this is probably fine...

      // Extrapolate track to the z position of the cluster
      const double tx = track.getOriginX() + track.getSlopeX() * cluster.getPosZ();
      const double ty = track.getOriginY() + track.getSlopeY() * cluster.getPosZ();

      // Compare the the cluster
      m_hResidualsX[iplane]->Fill(tx - cluster.getPosX());
      m_hResidualsY[iplane]->Fill(ty - cluster.getPosY());
    }

    // NOTE: reference residuals are exclusively to clusters in the track

    // Iterate through the reamaining devices (DUTs)
    size_t iglobal = -1;
    for (size_t ievent = 1; ievent < m_events.size(); ievent++) {
      const Storage::Event& event = *m_events[ievent];
      for (size_t iplane = 0; iplane < event.getNumPlanes(); iplane++) {
        const Storage::Plane& plane = event.getPlane(iplane);
        iglobal += 1;

        if (plane.getNumClusters() == 0) continue;

        // Find the cluster nearest to the track in the DUT
        const Storage::Cluster* best = 0;
        double bestDist2 = 0;
        for (size_t icluster = 0; icluster < plane.getNumClusters(); icluster++) {
          const Storage::Cluster& cluster = plane.getCluster(icluster);

          const double tx = track.getOriginX() + track.getSlopeX() * cluster.getPosZ();
          const double ty = track.getOriginY() + track.getSlopeY() * cluster.getPosZ();

          const double dist2 = 
              std::pow(cluster.getPosX() - tx, 2) +
              std::pow(cluster.getPosY() - ty, 2);

          if (!best || dist2 < bestDist2) {
            best = &cluster;
            bestDist2 = dist2;
          }
        }

        // Compute and store its residual
        const double tx = track.getOriginX() + track.getSlopeX() * best->getPosZ();
        const double ty = track.getOriginY() + track.getSlopeY() * best->getPosZ();
        m_hResidualsX[iplane]->Fill(tx - best->getPosX());
        m_hResidualsY[iplane]->Fill(ty - best->getPosY());
      }
    }
  }
}

size_t TrackResiduals::toGlobal(size_t idevice, size_t isensor) const {
  assert(!m_devices.empty());
  // Simple counting of sensor up to the correct device. Infrequently called,
  // so no need to cache or map indices.
  size_t iglobal = 0;
  for (size_t id = 0; id < idevice; id++)
    iglobal += m_devices[id]->getNumSensors();
  iglobal += isensor;
  return iglobal;
}

TH1D& TrackResiduals::getResidualX(size_t idevice, size_t isensor) const {
  const size_t iglobal = toGlobal(idevice, isensor);
  return *m_hResidualsX[iglobal];
}

TH1D& TrackResiduals::getResidualY(size_t idevice, size_t isensor) const {
  const size_t iglobal = toGlobal(idevice, isensor);
  return *m_hResidualsY[iglobal];
}

}

