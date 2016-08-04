#include "clustermaker.h"

#include <cassert>
#include <iostream>
#include <math.h>
#include <float.h>

#include "../storage/plane.h"
#include "../storage/cluster.h"
#include "../storage/hit.h"
#include "../storage/event.h"
#include "processors.h"

namespace Processors {

void ClusterMaker::addNeighbours(const Storage::Hit* hit, const  Storage::Plane* plane,
                                  Storage::Cluster* cluster)
{
  // Go through all hits
  for (unsigned int nhit = 0; nhit < plane->getNumHits(); nhit++)
  {
     Storage::Hit* compare = plane->getHit(nhit);

    // Continue if this hit is already clustered or if it is the one being considered
    if (compare->getCluster() || compare == hit) continue;

    // If a maximum separation has been defined in real coordinates, check now
    if (_maxSeparation > 0)
    {
      const double distX = compare->getPosX() - hit->getPosX();
      const double distY = compare->getPosY() - hit->getPosY();
      const double dist = sqrt(pow(distX, 2) + pow(distY, 2));
      if (dist > _maxSeparation) continue;
    }
    else
    {
      const int distX = compare->getPixX() - hit->getPixX();
      const int distY = compare->getPixY() - hit->getPixY();
      if (fabs(distX) > _maxSeparationX || fabs(distY) > _maxSeparationY)
        continue;
    }

    // Add this hit to the cluster we are making
    cluster->addHit(compare);

    // Continue adding neigbours of this hit into the cluster
    addNeighbours(compare, plane, cluster);
  }
}

void ClusterMaker::generateClusters(Storage::Event* event, unsigned int planeNum)
{
   Storage::Plane* plane = event->getPlane(planeNum);
  if (plane->getNumClusters() > 0)
    throw "ClusterMaker: clusters already exist for this hit";

  for (unsigned int nhit = 0; nhit < plane->getNumHits(); nhit++)
  {
     Storage::Hit* hit = plane->getHit(nhit);
     //std::cout << "addHit: " << nhit  << " isclustered: " <<hit->getCluster() << " isHit: " << hit->getIsHit() << std::endl;
    // If the hit isn't clustered, make a new cluster
    if (!hit->getCluster()) {
       Storage::Cluster* cluster = event->newCluster(planeNum);
      cluster->addHit(hit);
    }

    // Add neighbouring clusters to this hit (this is recursive)
     Storage::Cluster* lastCluster = plane->getCluster(plane->getNumClusters() - 1);
    assert(lastCluster && "ClusterMaker: hits didn't generate any clusters");
    addNeighbours(hit, plane, lastCluster);
  }

  // Recursive search has ended, finalize all the cluster information
  for (unsigned int i = 0; i < plane->getNumClusters(); i++)
    calculateCluster(plane->getCluster(i));
}

void ClusterMaker::calculateCluster(Storage::Cluster* cluster)
{
  const double rt12 = 1.0 / sqrt(12);
  double pixErrX = rt12;
  double pixErrY = rt12;

  double cogX = 0;
  double cogY = 0;
  double weight = 0;

  for (unsigned int nhit = 0; nhit < cluster->getNumHits(); nhit++)
  {
    const Storage::Hit* hit = cluster->getHit(nhit);
    const double value = (hit->getValue() > 0) ? hit->getValue() : 1;
    cogX += hit->getPixX() * value;
    cogY += hit->getPixY() * value;
    //added matevz 20141130
    //weight += hit->getValue();
    weight += value;
  }

  cogX /= weight;
  cogY /= weight;

  double stdevX = 0;
  double stdevY = 0;
  double weight2 = 0;

  for (unsigned int nhit = 0; nhit < cluster->getNumHits(); nhit++)
  {
    const Storage::Hit* hit = cluster->getHit(nhit);
    const double value = (hit->getValue() > 0) ? hit->getValue() : 1;
    stdevX += pow(value * (hit->getPixX() - cogX), 2.0);
    stdevY += pow(value * (hit->getPixY() - cogY), 2.0);
    weight2 += value*value;
  }

  cogX += 0.5;
  cogY += 0.5;

  assert(cluster->getNumHits() > 0 && "ClusterMaker: this is really bad");

  stdevX = sqrt(stdevX / weight2);
  stdevY = sqrt(stdevY / weight2);

  const double errX = (stdevX) ? stdevX : pixErrX;
  const double errY = (stdevY) ? stdevY : pixErrY;

  cluster->setPix(cogX, cogY);
  cluster->setPixErr(errX, errY);
}

ClusterMaker::ClusterMaker(unsigned int maxSeparationX, unsigned int maxSeparationY,
                           double maxSeparation) :
  _maxSeparationX(maxSeparationX), _maxSeparationY(maxSeparationY),
  _maxSeparation(maxSeparation)
{
  if (_maxSeparation < 0)
    throw "ClusterMaker: max separation must be positive";
}

}
