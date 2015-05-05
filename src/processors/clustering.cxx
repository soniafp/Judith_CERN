#include <iostream>
#include <vector>
#include <stdexcept>
#include <list>
#include <stack>
#include <cmath>

#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/event.h"
#include "processors/clustering.h"

namespace Processors {

Clustering::Clustering() :
    m_maxRows(1),
    m_maxCols(1) {}

void Clustering::clusterSeed(
    Storage::Hit& seed,
    std::list<Storage::Hit*>& hits,
    std::list<Storage::Hit*>& clustered) {
  // List of hist for which to search for neighbours
  std::stack<Storage::Hit*> search;

  clustered.push_back(&seed);  // add seed to the cluster
  search.push(&seed);  // add seed to neighbour search

  // For each clustered hit, search for its neighbours
  while (!search.empty()) {
    // Target hit about which to find neighbours
    const Storage::Hit* target = search.top();
    search.pop();

    // Look through remaining unclustered hits for neighbouring ones
    for (std::list<Storage::Hit*>::iterator it = hits.begin();
        it != hits.end(); ++it) {
      Storage::Hit* hit = *it;  // candidate neighbour
      // If this hit isn't close enough to the target, move on
      if (std::abs((int)hit->getPixX()-(int)target->getPixX()) > m_maxRows ||
          std::abs((int)hit->getPixY()-(int)target->getPixY()) > m_maxCols)
        continue;
      clustered.push_back(hit);
      // Erease from the list of unclustered list. This returns the next
      // iterator. Move it back so that the loop moves forward correctly.
      it = --(hits.erase(it));
      // This hit must now also be included in the neighbour search
      search.push(hit);
    }
  }
}

void Clustering::buildCluster(
    Storage::Cluster& cluster,
    std::list<Storage::Hit*>& clustered) {
  const size_t nhits = clustered.size();
  cluster.reserveHits(nhits);

  // Variables for weighted incremental variance computation
  // http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
  double sumw = 0;
  double meanX = 0;
  double meanY = 0;
  double m2X = 0;
  double m2Y = 0;

  for (std::list<Storage::Hit*>::iterator it = clustered.begin();
      it != clustered.end(); ++it) {
    // Add the hit to the cluster
    Storage::Hit& hit = **it;
    cluster.addHit(hit);

    // Weight the hits by their value, if they have values (value of 0 should
    // not be an allowed value)
    const double weight = (hit.getValue()>0) ? hit.getValue() : 1;
    const double tmp = weight + sumw;

    const double deltaX = hit.getPixX() - meanX;
    const double rX = deltaX * weight / tmp;
    meanX += rX;
    m2X += sumw * deltaX * rX;

    const double deltaY = hit.getPixY() - meanY;
    const double rY = deltaY * weight / tmp;
    meanY += rY;
    m2Y += sumw * deltaY * rY;

    sumw = tmp;
  }

  // Set the cluster mean
  cluster.setPix(meanX, meanY);

  if (nhits == 1)
    // For a single hit cluster, use the RMS of a flat distribution
    cluster.setPixErr(1./std::sqrt(12), 1./std::sqrt(12));
  else if (nhits > 1)
    // Otherwise, use the sample RMS
    cluster.setPixErr(
        std::sqrt(m2X/sumw * nhits/(double)(nhits-1)),
        std::sqrt(m2Y/sumw * nhits/(double)(nhits-1)));
}

void Clustering::process(Storage::Event& event) {
  const size_t nplanes = event.getNumPlanes();

  if (event.getNumClusters())
    throw std::runtime_error("Clustering::process: event is already clustered");

  for (size_t iplane = 0; iplane < nplanes; iplane++) {
    const Storage::Plane& plane = event.getPlane(iplane);
    if (plane.getNumHits() == 0) continue;

    // Store all hits in this plane in a list
    std::list<Storage::Hit*> hits(
        plane.getHits().cbegin(), plane.getHits().cend());

    while (!hits.empty()) {
      // Use the last hit as the seed hit, and remove from hits to cluster
      Storage::Hit& seed = *hits.back();
      hits.pop_back();
      // Build the cluster, removing all clustered hits along the way
      std::list<Storage::Hit*> clustered;
      clusterSeed(seed, hits, clustered);
      // Compute the cluster's values and store in cluster object
      Storage::Cluster& cluster = event.newCluster(iplane);
      buildCluster(cluster, clustered);
    }
  }
}

void Clustering::setMaxPixels(unsigned cols, unsigned rows) {
  m_maxCols = cols;
  m_maxRows = rows;
}

}

