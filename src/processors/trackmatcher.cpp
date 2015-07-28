#include "trackmatcher.h"

#include <cassert>

#include "../storage/event.h"
#include "../storage/track.h"
#include "../storage/plane.h"
#include "../storage/cluster.h"
#include "../mechanics/device.h"
#include "../processors/processors.h"

namespace Processors {

void TrackMatcher::matchTracksToClusters(Storage::Event* trackEvent,
                                         Storage::Plane* clustersPlane,
                                         const Mechanics::Sensor* clustersSensor)
{
  // Look for matches for all tracks
  for (unsigned int ntrack = 0; ntrack < trackEvent->getNumTracks(); ntrack++)
  {
    Storage::Track* track = trackEvent->getTrack(ntrack);

    // Find the nearest cluster
    double nearestDist = 0;
    Storage::Cluster* match = 0;

    for (unsigned int ncluster = 0; ncluster < clustersPlane->getNumClusters(); ncluster++)
    {
      Storage::Cluster* cluster = clustersPlane->getCluster(ncluster);

      double x = 0, y = 0, errx = 0, erry = 0;
      trackClusterDistance(track, cluster, clustersSensor, x, y, errx, erry);

      const double dist = sqrt(pow(x / errx, 2) + pow(y / erry, 2));

      if (!match || dist < nearestDist)
      {
        nearestDist = dist;
        match = cluster;
      }
    }

    // If is a match, store this in the event
    if (match)
    {
      track->addMatchedCluster(match);
      match->setMatchedTrack(track);
      match->setMatchDistance(nearestDist);
    }
  }
}

void TrackMatcher::matchClustersToTracks(Storage::Event* trackEvent,
                                         Storage::Plane* clustersPlane,
                                         const Mechanics::Sensor* clustersSensor)
{
  for (unsigned int ncluster = 0; ncluster < clustersPlane->getNumClusters(); ncluster++)
  {
    Storage::Cluster* cluster = clustersPlane->getCluster(ncluster);

    // Find the nearest track
    double nearestDist = 0;
    Storage::Track* match = 0;

    // Look for matches for all tracks
    for (unsigned int ntrack = 0; ntrack < trackEvent->getNumTracks(); ntrack++)
    {
      Storage::Track* track = trackEvent->getTrack(ntrack);

      double x = 0, y = 0, errx = 0, erry = 0;
      trackClusterDistance(track, cluster, clustersSensor, x, y, errx, erry);

      const double dist = sqrt(pow(x / errx, 2) + pow(y / erry, 2));

      if (!match || dist < nearestDist)
      {
        nearestDist = dist;
        match = track;
      }
    }

    // If is a match, store this in the event
    if (match)
    {
      match->addMatchedCluster(cluster);
      cluster->setMatchedTrack(match);
      cluster->setMatchDistance(nearestDist);
    }
  }
}

void TrackMatcher::matchEvent(Storage::Event* refEvent,
                              Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "TrackMatching: null event and/or device");
  assert(dutEvent->getNumPlanes() == _device->getNumSensors() &&
         "TrackMatching: plane / sensor mis-match");

  // Look for a tracks to clusters in each plane
  for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
  {
    Storage::Plane* plane = dutEvent->getPlane(nplane);
    Mechanics::Sensor* sensor = _device->getSensor(nplane);

    // If there are more cluster than tracks, match each track to one cluster
    if (plane->getNumClusters() >= refEvent->getNumTracks())
      matchTracksToClusters(refEvent, plane, sensor);
    else
      matchClustersToTracks(refEvent, plane, sensor);
  }
}

TrackMatcher::TrackMatcher(const Mechanics::Device* device) :
  _device(device)
{  }

}
