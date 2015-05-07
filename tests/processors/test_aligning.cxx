#include <iostream>
#include <stdexcept>
#include <cmath>

#include "storage/event.h"
#include "storage/cluster.h"
#include "storage/hit.h"
#include "mechanics/device.h"
#include "processors/clustering.h"
#include "processors/aligning.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_aligning() {
  const size_t nplanes = 2;

  Mechanics::Device device(nplanes);
  device.setOffX(1);
  device.setOffY(-2);
  device.setOffZ(3);
  device.setRotX(.1);
  device.setRotY(-.2);
  device.setRotZ(.3);

  device.getSensor(0).m_rowPitch = 1;
  device.getSensor(0).m_colPitch = 2;
  device.getSensor(0).setOffX(-.11);
  device.getSensor(0).setOffX(.12);
  device.getSensor(0).setOffX(-.13);
  device.getSensor(0).setRotX(-1.11);
  device.getSensor(0).setRotX(1.12);
  device.getSensor(0).setRotX(-1.13);

  device.getSensor(0).m_rowPitch = 3;
  device.getSensor(0).m_colPitch = 4;
  device.getSensor(1).setOffX(-.51);
  device.getSensor(1).setOffX(.15);
  device.getSensor(1).setOffX(-.53);
  device.getSensor(1).setRotX(-1.51);
  device.getSensor(1).setRotX(1.15);
  device.getSensor(1).setRotX(-1.53);

  Storage::Event event(nplanes);

  Storage::Hit& hit0 = event.newHit(0);
  hit0.setPix(1,1);

  Storage::Hit& hit1 = event.newHit(1);
  hit1.setPix(2,2);

  Processors::Clustering clustering;
  clustering.process(event);
  
  Processors::Aligning aligning(device);
  aligning.process(event);

  double x0, y0, z0, x1, y1, z1;

  device.pixelToSpace(1, 1, 0, x0, y0, z0);
  device.pixelToSpace(2, 2, 1, x1, y1, z1);

  const Storage::Cluster& cluster0 = event.getCluster(0);
  const Storage::Cluster& cluster1 = event.getCluster(1);

  if (hit0.getPosX() != x0 ||
      hit0.getPosY() != y0 ||
      hit0.getPosZ() != z0 ||
      hit1.getPosX() != x1 ||
      hit1.getPosY() != y1 ||
      hit1.getPosZ() != z1) {
    std::cerr << "Hits not correctly aligned" << std::endl;
    return -1;
  }

  if (cluster0.getPosX() != x0 ||
      cluster0.getPosY() != y0 ||
      cluster0.getPosZ() != z0 ||
      cluster1.getPosX() != x1 ||
      cluster1.getPosY() != y1 ||
      cluster1.getPosZ() != z1) {
    std::cerr << "Clusters not correctly aligned" << std::endl;
    return -1;
  }

  // Transform the point 1 sigma away, and then get distance from transformed 
  // central value
  device.pixelToSpace(1+cluster0.getPixErrX(), 1+cluster0.getPixErrY(), 0, x1, y1, z1);
  x1 = std::fabs(x1-x0);
  y1 = std::fabs(y1-y0);
  z1 = std::fabs(z1-z0);

  if (cluster0.getPosErrX() != x1 ||
      cluster0.getPosErrY() != y1 ||
      cluster0.getPosErrZ() != z1) {
    std::cerr << "Clusters error not correctly aligned" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_aligning()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
