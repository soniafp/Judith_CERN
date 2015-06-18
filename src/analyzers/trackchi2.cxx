#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>

#include "storage/event.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "storage/track.h"
#include "mechanics/device.h"
#include "mechanics/sensor.h"
#include "analyzers/trackchi2.h"

namespace Analyzers {

void TrackChi2::initialize() {
  if (m_devices.size() != 1 && m_devices.size() != 2)
    throw std::runtime_error(
        "TrackChi2::initialize: only works for 1 or 2 devices");

  m_numRefPlanes = m_devices[0]->getNumSensors();
  if (m_devices.size() > 1)
    m_numDUTPlanes = m_devices[1]->getNumSensors();
}

void TrackChi2::process() {
  // Keep track of the reference event (with all reference planes)
  assert(m_ndevices > 0 && "ensures > 0 events");
  const Storage::Event& refEvent = *m_events[0];

  // Make alignment track objects from reference tracks
  for (size_t itrack = 0; itrack < refEvent.getNumTracks(); itrack++) {
    const Storage::Track& track = refEvent.getTrack(itrack);

    m_tracks.push_back(Track(m_numRefPlanes, m_numDUTPlanes));

    // Make alignment cluster objects from track clusters and link to the track
    for (size_t icluster = 0; icluster < track.getNumClusters(); icluster++) {
      const Storage::Cluster& cluster = track.getCluster(icluster);

      assert(cluster.fetchPlane() && "Can't have a cluster with no plane");
      const size_t iplane = cluster.fetchPlane()->getPlaneNum();

      m_clusters.push_back(Cluster(
          m_devices[0]->getSensorConst(iplane),
          cluster.getPixX(),
          cluster.getPixY(),
          cluster.getPixErrX(),
          cluster.getPixErrY()));
      m_icluster += 1;

      // Associate this alignment cluster to its alignment track
      m_tracks.back().constituents.push_back(m_icluster-1);
    }

    // Move onto the DUT to look for matches, if one is given
    if (m_ndevices == 1) continue;
    const Storage::Event& dutEvent = *m_events[1];

    // Find matching clusters in the DUT if one is present
    for (size_t iplane = 0; iplane < dutEvent.getNumPlanes(); iplane++) {
      const Storage::Plane& plane = dutEvent.getPlane(iplane);

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

      m_clusters.push_back(Cluster(
          m_devices[1]->getSensorConst(iplane),
          best->getPixX(),
          best->getPixY(),
          best->getPixErrX(),
          best->getPixErrY()));
      m_icluster += 1;
      m_tracks.back().matches.push_back(m_icluster-1);
    }
  }
}

}

