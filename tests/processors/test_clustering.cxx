#include <iostream>
#include <stdexcept>
#include <cmath>

#include "storage/event.h"
#include "storage/cluster.h"
#include "storage/hit.h"
#include "processors/clustering.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_clustering() {
  const size_t nplanes = 1;
  Storage::Event event(nplanes);

  Storage::Hit& hit00 = event.newHit(0);
  Storage::Hit& hit01 = event.newHit(0);
  Storage::Hit& hit10 = event.newHit(0);
  Storage::Hit& hit11 = event.newHit(0);
  Storage::Hit& hit12 = event.newHit(0);
  Storage::Hit& hit20 = event.newHit(0);

  hit00.setPix(0, 0);
  hit01.setPix(1, 1);

  hit10.setPix(3, 0);
  hit11.setPix(3, 1);
  hit12.setPix(4, 1);

  hit20.setPix(5, 5);

  const double mean0X = (0+1)/2.;
  const double rms0X = std::sqrt((
      std::pow(0-mean0X, 2) +
      std::pow(1-mean0X, 2)) / (2-1));
  const double mean0Y = (0+1)/2.;
  const double rms0Y = std::sqrt((
      std::pow(0-mean0Y, 2) +
      std::pow(1-mean0Y, 2)) / (2-1));

  const double mean1X = (3+3+4)/3.;
  const double rms1X = std::sqrt((
      std::pow(3-mean1X, 2) +
      std::pow(3-mean1X, 2) +
      std::pow(4-mean1X, 2)) / (3-1));
  const double mean1Y = (0+1+1)/3.;
  const double rms1Y = std::sqrt((
      std::pow(0-mean1Y, 2) +
      std::pow(1-mean1Y, 2) +
      std::pow(1-mean1Y, 2)) / (3-1));

  const double mean2X = 5;
  const double rms2X = 1./std::sqrt(12);
  const double mean2Y = 5;
  const double rms2Y = 1./std::sqrt(12);

  Processors::Clustering clustering;
  clustering.process(event);

  if (event.getNumClusters() != 3) {
    std::cerr << "Processors::Clustering: multiplicity failed" << std::endl;
    return -1;
  }

  if (!approxEqual(event.getCluster(0).getPixX(), mean2X) ||
      !approxEqual(event.getCluster(0).getPixY(), mean2Y) ||
      !approxEqual(event.getCluster(1).getPixX(), mean1X) ||
      !approxEqual(event.getCluster(1).getPixY(), mean1Y) ||
      !approxEqual(event.getCluster(2).getPixX(), mean0X) ||
      !approxEqual(event.getCluster(2).getPixY(), mean0Y)) {
    std::cerr << "Processors::Clustering: mean failed" << std::endl;
    return -1;
  }

  if (!approxEqual(event.getCluster(0).getPixErrX(), rms2X) ||
      !approxEqual(event.getCluster(0).getPixErrY(), rms2Y) ||
      !approxEqual(event.getCluster(1).getPixErrX(), rms1X) ||
      !approxEqual(event.getCluster(1).getPixErrY(), rms1Y) ||
      !approxEqual(event.getCluster(2).getPixErrX(), rms0X) ||
      !approxEqual(event.getCluster(2).getPixErrY(), rms0Y)) {
    std::cerr << "Processors::Clustering: RMS failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_clustering()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
