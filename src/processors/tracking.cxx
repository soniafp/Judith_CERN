#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <list>
#include <cmath>

#include "utils.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/track.h"
#include "storage/event.h"
#include "processors/tracking.h"

namespace Processors {

void Tracking::buildTrack(
    Storage::Track& track,
    const std::list<Storage::Cluster*>& clusters) {
  const size_t nclusters = clusters.size();
  std::vector<double> x(nclusters, 0);
  std::vector<double> xe(nclusters, 0);
  std::vector<double> y(nclusters, 0);
  std::vector<double> ye(nclusters, 0);
  std::vector<double> z(nclusters, 0);

  size_t icluster = 0;
  for (std::list<Storage::Cluster*>::const_iterator it = clusters.begin();
      it != clusters.end(); ++it) {
    Storage::Cluster& cluster = **it;
    track.addCluster(cluster);
    x[icluster] = cluster.getPosX();
    xe[icluster] = cluster.getPosErrX();
    y[icluster] = cluster.getPosY();
    ye[icluster] = cluster.getPosErrY();
    z[icluster] = cluster.getPosZ();
    icluster += 1;
  }

  double xp0 = 0;  // x intercept
  double xp1 = 0;  // x slope
  double xp0e = 0;  // x intercept error
  double xp1e = 0;  // x slope error
  double xcov = 0;  // x intercept slope covariance
  double xchi2 = 0;  // x fit chi^2

  double yp0 = 0;
  double yp1 = 0;
  double yp0e = 0;
  double yp1e = 0;
  double ycov = 0;
  double ychi2 = 0;

  Utils::linearFit(
    nclusters, &z[0], &x[0], &xe[0], xp0, xp1, xp0e, xp1e, xcov, xchi2);
  Utils::linearFit(
    nclusters, &z[0], &y[0], &ye[0], yp0, yp1, yp0e, yp1e, ycov, ychi2);

  track.setSlope(xp1, yp1);
  track.setSlopeErr(xp1e, yp1e);
  track.setOrigin(xp0, yp0);
  track.setOriginErr(xp0e, yp0e);
  track.setCovariance(xcov, ycov);
  // 2n d.o.f. - 2 fixed
  track.setChi2((xchi2+ychi2)/(2*nclusters-2));
}

void Tracking::process() {
  // Processor initializes with only 1 device, so process runs on only 1 event
  assert(m_events.size() == 1 && "More than 1 device being tracked");
  Storage::Event& event = *m_events[0];

  // Consistency check since some allocation has been done to accomodate a
  // given number of planes
  if (event.getNumPlanes() != m_nplanes)
    throw std::runtime_error("Tracking::process: wrong number of planes");

  // Track candidates need to keep track of the last plane on which they found
  // a cluster, and the list of found clusters
  struct Tracklet {
    // Index of last plane on which the track found a cluster
    size_t lastPlane;
    // List of clusters in this track
    std::list<Storage::Cluster*> clusters;
    Tracklet() : lastPlane(0), clusters() {}
  };

  // List won't be moving Tracklet objects, which are light anyway, so no real
  // performance issues
  std::list<Tracklet> tracklets;

  const size_t minClusters = (m_minClusters>3) ? m_minClusters : 3;

  for (size_t iplane = 0; iplane < m_nplanes; iplane++) {
    const Storage::Plane& plane = event.getPlane(iplane);

    // Try to add all clusters to tracks
    for (size_t icluster = 0; icluster < plane.getNumClusters(); icluster++) {
      Storage::Cluster& cluster = plane.getCluster(icluster);
      // Iterator to the track matching this cluster
      std::list<Tracklet>::iterator match = tracklets.end();
      // Boolean remembers if the cluster was matched (even if match has been
      // erased)
      bool matched = false;

      // Try to associate this cluster to all track candidates
      for (std::list<Tracklet>::iterator it = tracklets.begin();
          it != tracklets.end(); ++it) {
        Tracklet& tracklet = *it;

        // Information about the last cluster in the candidate
        const size_t iprev = tracklet.lastPlane;
        const double prevx = tracklet.clusters.back()->getPosX();
        const double prevy = tracklet.clusters.back()->getPosY();

        // Information about this cluster
        const size_t icurr = iplane;
        const double currx = cluster.getPosX();
        const double curry = cluster.getPosY();
        
        // The RMS of the distance of clusters from iprev plane to icurr plane.
        const double scalex = m_transitionsX[iprev*m_nplanes+icurr];
        const double scaley = m_transitionsY[iprev*m_nplanes+icurr];

        // The distance from this cluster to the track, scaled
        const double distx = std::fabs((currx-prevx)/scalex);
        const double disty = std::fabs((curry-prevy)/scaley);
        const double dist = std::sqrt(distx*distx + disty*disty);

        // The tracklet matches the cluster, and the cluster is unmatched
        if (dist <= m_radius && !matched) {
          // Cluster is a candidate, add it to the track
          tracklet.lastPlane = iplane;
          tracklet.clusters.push_back(&cluster);
          // Remember which track matched the cluster
          match = it;
          matched = true;
        }

        // The tracklet matches the cluster, but the cluster is already in 
        // another track
        else if (dist <= m_radius && matched) {
          // If the cluster has a matched tracklet, erase it. Note: this must
          // be done before erasing the current tracklet, since the iterator
          // might be moved back to onto this one
          if (match != tracklets.end()) {
            tracklets.erase(match);
            match = tracklets.end();
          }
          // Current tracket is also invalidated. Erase it. Get the iterator to
          // the next tracklet, but decrement it so that the next for brings 
          // the loop back to it
          it = tracklets.erase(it); --it;
          // Move onto the next tracklet (can't be trying to use tracklet)
          continue;
        }
      }  // tracklet loop

      // Unmatched clusters that can still seed a full track should do so
      if (!matched && m_nplanes-iplane >= minClusters) {
        tracklets.push_back(Tracklet());
        tracklets.back().lastPlane = iplane;
        tracklets.back().clusters.push_back(&cluster);
      }
    }  // cluster loop

    // Now remove tracklets which can no longer be completed
    for (std::list<Tracklet>::iterator it = tracklets.begin();
        it != tracklets.end(); ++it) {
      Tracklet& tracklet = *it;
      const size_t nremains = m_nplanes - (iplane+1);
      if (tracklet.clusters.size() + nremains >= minClusters)
        continue;
      it = --(tracklets.erase(it));
    }
  }  // plane loop

  // Build tracks from the tracklets
  for (std::list<Tracklet>::iterator it = tracklets.begin();
      it != tracklets.end(); ++it) {
    Tracklet& tracklet = *it;
    Storage::Track& track = event.newTrack();
    buildTrack(track, tracklet.clusters);
  }
}

void Tracking::setTransitionX(size_t from, size_t to, double scale) {
  m_transitionsX[from*m_nplanes+to] = scale;
}

void Tracking::setTransitionY(size_t from, size_t to, double scale) {
  m_transitionsY[from*m_nplanes+to] = scale;
}

}

