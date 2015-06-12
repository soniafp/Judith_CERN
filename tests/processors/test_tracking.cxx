#include <iostream>
#include <stdexcept>
#include <cmath>

#include "utils.h"
#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/event.h"
#include "storage/track.h"
#include "processors/tracking.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

void newCluster(Storage::Event& event, size_t iplane, double posx, double posy=0) {
  event.newCluster(iplane);
  event.getPlane(iplane).getClusters().back()->setPos(posx, posy, iplane);
}

int test_association() {
  const size_t nplanes = 4;
  Storage::Event event(nplanes);

  // Patter of clusters: | are sensors, X are hits, V^ make a hit between
  // 0  |  |  x  x  : never seeded
  // 1  |  x  |  x  : fails 3rd (insufficient)
  // 2  x  x  x  x  : 1st track
  // 3  x  x  x  |  : 2nd track
  // 4  x  x  |  x  : 3rd track
  // 5  |  x  x  x  : 4th track
  // 6  x  x  v  x  : fails on 3rd (overlap)
  // 7  x  x  ^  x  : fails on 3rd (overlap)
  // 8  x  |  |  v  : fails on 3nd (insufficient)
  // 9  |  x  x  ^  : 5th track

  size_t iplane = 0;
  newCluster(event, iplane, 2);
  newCluster(event, iplane, 3);
  newCluster(event, iplane, 4);
  newCluster(event, iplane, 6);
  newCluster(event, iplane, 7);
  newCluster(event, iplane, 8);

  iplane = 1;
  newCluster(event, iplane, 1);
  newCluster(event, iplane, 2);
  newCluster(event, iplane, 3);
  newCluster(event, iplane, 4);
  newCluster(event, iplane, 5);
  newCluster(event, iplane, 6);
  newCluster(event, iplane, 7);
  newCluster(event, iplane, 9);

  iplane = 2;
  newCluster(event, iplane, 0);
  newCluster(event, iplane, 2);
  newCluster(event, iplane, 3);
  newCluster(event, iplane, 5);
  newCluster(event, iplane, 6.5);
  newCluster(event, iplane, 9);

  iplane = 3;
  newCluster(event, iplane, 0);
  newCluster(event, iplane, 1);
  newCluster(event, iplane, 2);
  newCluster(event, iplane, 4);
  newCluster(event, iplane, 5);
  newCluster(event, iplane, 6);
  newCluster(event, iplane, 7);
  newCluster(event, iplane, 8.5);

  Processors::Tracking tracking(nplanes);
  tracking.m_minClusters = 3;

  // Set the search radius to 5: clusters within 5 units of transition scale
  // are grouped into a track. Set the transition scale between each sensor to
  // 2/3 of 1/5. This means that clusters within 0.67 will be grouped, allowing
  // for sharing of those at 0.5 form one another.
  tracking.m_radius = 5;
  for (size_t i = 0; i < nplanes; i++)
    for (size_t j = 0; j < nplanes; j++)
      tracking.setTransitionX(i, j, 2/3.*1/5.);

  tracking.execute(event);

  if (event.getNumTracks() != 5) {
    std::cerr << "Processors::Tracking: multiplicity failed" << std::endl;
    return -1;
  }

  if (event.getTrack(0).getNumClusters() != 4 ||
      &event.getTrack(0).getCluster(0) != &event.getPlane(0).getCluster(0) ||
      &event.getTrack(0).getCluster(1) != &event.getPlane(1).getCluster(1) ||
      &event.getTrack(0).getCluster(2) != &event.getPlane(2).getCluster(1) ||
      &event.getTrack(0).getCluster(3) != &event.getPlane(3).getCluster(2)) {
    std::cerr << "Processors::Tracking: track 1 clusters not as expected" << std::endl;
    return -1;
  }

  if (event.getTrack(1).getNumClusters() != 3 ||
      &event.getTrack(1).getCluster(0) != &event.getPlane(0).getCluster(1) ||
      &event.getTrack(1).getCluster(1) != &event.getPlane(1).getCluster(2) ||
      &event.getTrack(1).getCluster(2) != &event.getPlane(2).getCluster(2)) {
    std::cerr << "Processors::Tracking: track 2 clusters not as expected" << std::endl;
    return -1;
  }

  if (event.getTrack(2).getNumClusters() != 3 ||
      &event.getTrack(2).getCluster(0) != &event.getPlane(0).getCluster(2) ||
      &event.getTrack(2).getCluster(1) != &event.getPlane(1).getCluster(3) ||
      &event.getTrack(2).getCluster(2) != &event.getPlane(3).getCluster(3)) {
    std::cerr << "Processors::Tracking: track 3 clusters not as expected" << std::endl;
    return -1;
  }

  if (event.getTrack(3).getNumClusters() != 3 ||
      &event.getTrack(3).getCluster(0) != &event.getPlane(1).getCluster(4) ||
      &event.getTrack(3).getCluster(1) != &event.getPlane(2).getCluster(3) ||
      &event.getTrack(3).getCluster(2) != &event.getPlane(3).getCluster(4)) {
    std::cerr << "Processors::Tracking: track 4 clusters not as expected" << std::endl;
    return -1;
  }

  if (event.getTrack(4).getNumClusters() != 3 ||
      &event.getTrack(4).getCluster(0) != &event.getPlane(1).getCluster(7) ||
      &event.getTrack(4).getCluster(1) != &event.getPlane(2).getCluster(5) ||
      &event.getTrack(4).getCluster(2) != &event.getPlane(3).getCluster(7)) {
    std::cerr << "Processors::Tracking: track 5 clusters not as expected" << std::endl;
    return -1;
  }

  return 0;
}

int test_minClusters() {
  const size_t nplanes = 4;
  Storage::Event event(nplanes);

  newCluster(event, 0, 0);
  newCluster(event, 1, 0);
  newCluster(event, 3, 0);

  newCluster(event, 0, 2);
  newCluster(event, 1, 2);
  newCluster(event, 2, 2);
  newCluster(event, 3, 2);

  Processors::Tracking tracking(nplanes);
  tracking.m_minClusters = 4;
  // Note: clusters within 1 of each other will be grouped (default scales: 1)
  tracking.m_radius = 1;

  tracking.execute(event);

  if (event.getNumTracks() != 1 ||
      event.getTrack(0).getNumClusters() != 4) {
    std::cerr << "Processors::Tracking: min clusters failed" << std::endl;
    return -1;
  }

  return 0;
}

int test_radius() {
  const size_t nplanes = 4;
  Storage::Event event(nplanes);

  // Example of transitions and radius:
  // search radius is sqrt(2)
  // distance in x between two clusters is 5
  // distance in y between two clusters is 3
  // transition in x between the two planes is 10 (rms of residuals)
  // transition in y between the two planes is 5
  // sqrt((5/10)^2 + (3/5)^2) < sqrt(2), so the clusters are matched

  newCluster(event, 0, 0, 0);  // seed cluster

  newCluster(event, 1, .9, 9);  // just within 0->1 transition
  newCluster(event, 1, 1.1, 11);   // just oustside transition (won't kill track)

  newCluster(event, 2, 2.1, 21);  // outside 1->2 transition, won't be included 

  newCluster(event, 3, 2.8, 28);  // just within 1->3 transition
  newCluster(event, 3, 3.2, 32);  // just outside transition

  Processors::Tracking tracking(nplanes);
  tracking.m_minClusters = 3;
  tracking.m_radius = std::sqrt(2);

  // 0->1 has difference 1 and 10, so normalize by that
  tracking.setTransitionX(0, 1, 1);
  tracking.setTransitionY(0, 1, 10);

  // 1->2 has 1 and 10 again, normalize by that
  tracking.setTransitionX(1, 2, 1);
  tracking.setTransitionY(1, 2, 10);

  // 1->3 has 2 and 20
  tracking.setTransitionX(1, 3, 2);
  tracking.setTransitionY(1, 3, 20);

  // Default transitions are 1, which won't be enough to make any other 3
  // cluster tracks, so don't bother

  tracking.execute(event);

  if (event.getNumTracks() != 1 ||
      event.getTrack(0).getNumClusters() != 3) {
    std::cerr << "Processors::Tracking: radius or transitions failed" << std::endl;
    return -1;
  }

  return 0;
}

int test_values() {
  const size_t nplanes = 3;
  Storage::Event event(nplanes);

  // Setup a 3 cluster track with some slope, chi2 ...
  newCluster(event, 0, -0.10, -0.20);
  event.getClusters().back()->setPosErr(.1, .2, 1);
  newCluster(event, 1, +0.05, -0.05);
  event.getClusters().back()->setPosErr(.2, .3, 1);
  newCluster(event, 2, +0.10, +0.20);
  event.getClusters().back()->setPosErr(.4, .5, 1);

  Processors::Tracking tracking(nplanes);
  tracking.m_minClusters = 3;
  // Note: radius much larger than distances, so track should build
  tracking.m_radius = 1;
  tracking.execute(event);

  if (event.getNumTracks() != 1) {
    std::cerr << "Processors::Tracking: problem tracking x and y" << std::endl;
    return -1;
  }

  // Do the linear regression explicitely

  double x[3] = { -0.10, +0.05, +0.10 };
  double xe[3] = { .1, .2, .4 };
  double y[3] = { -0.20, -0.05, +0.20 };
  double ye[3] = { .2, .3, .5 };
  double z[3] = { 0, 1, 2 };

  double a, ae, b, be, chi2, cov;
  double sumChi2 = 0;

  Utils::linearFit(3, z, x, xe, a, ae, b, be, chi2, cov);
  sumChi2 += chi2;

  if (!approxEqual(event.getTrack(0).getOriginX(), b) ||
      !approxEqual(event.getTrack(0).getOriginErrX(), be),
      !approxEqual(event.getTrack(0).getSlopeX(), a),
      !approxEqual(event.getTrack(0).getSlopeErrX(), ae),
      !approxEqual(event.getTrack(0).getCovarianceX(), cov)) {
    std::cerr << "Processors::Tracking: track values in x don't match" << std::endl;
    return -1;
  }

  Utils::linearFit(3, z, y, ye, a, ae, b, be, chi2, cov);
  sumChi2 += chi2;

  if (!approxEqual(event.getTrack(0).getOriginY(), b) ||
      !approxEqual(event.getTrack(0).getOriginErrY(), be),
      !approxEqual(event.getTrack(0).getSlopeY(), a),
      !approxEqual(event.getTrack(0).getSlopeErrY(), ae),
      !approxEqual(event.getTrack(0).getCovarianceY(), cov)) {
    std::cerr << "Processors::Tracking: track values in y don't match" << std::endl;
    return -1;
  }

  if (!approxEqual(event.getTrack(0).getChi2(), sumChi2/(2.*3-2), 1e-5)) {
    std::cerr << "Processors::Tracking: track chi2 doesn't match" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_association()) != 0) return retval;
    if ((retval = test_minClusters()) != 0) return retval;
    if ((retval = test_radius()) != 0) return retval;
    if ((retval = test_values()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
