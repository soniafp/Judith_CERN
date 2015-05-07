#include <iostream>
#include <stdexcept>
#include <cmath>

#include "storage/hit.h"
#include "storage/cluster.h"
#include "storage/plane.h"
#include "storage/event.h"
#include "mechanics/device.h"
#include "processors/aligning.h"

namespace Processors {

Aligning::Aligning(const Mechanics::Device& device) :
    m_device(device) {}

void Aligning::process(Storage::Event& event) {
  // Ensure the event has the same planes as the device
  if (event.getNumPlanes() != m_device.getNumSensors())
    throw std::runtime_error("Aligning::process: plane/sensor mismatch");

  // Get hits and clusters one plane at a time, and apply alignment
  for (size_t iplane = 0; iplane < event.getNumPlanes(); iplane++) {
    const Storage::Plane& plane = event.getPlane(iplane);

    // Iterate through all hits in this place
    const std::vector<Storage::Hit*>& hits = plane.getHits();
    for (std::vector<Storage::Hit*>::const_iterator it = hits.begin();
        it != hits.end(); ++it) {
      double x, y, z;
      Storage::Hit& hit = **it;
      // Ask the device to apply first local, then global transformation to
      // the hit's pixel coordinates
      m_device.pixelToSpace(
          hit.getPixX(), hit.getPixY(), iplane, x, y, z);
      hit.setPos(x, y, z);
    }

    // Iterate through all clusters in this plane
    const std::vector<Storage::Cluster*>& clusters = plane.getClusters();
    for (std::vector<Storage::Cluster*>::const_iterator it = clusters.begin();
        it != clusters.end(); ++it) {
      double x, y, z;
      Storage::Cluster& cluster = **it;
      m_device.pixelToSpace(
          cluster.getPixX(), cluster.getPixY(), iplane, x, y, z);
      cluster.setPos(x, y, z);
      // Compute also the positions shifted by 1 sigma
      double ex, ey, ez;
      m_device.pixelToSpace(
          cluster.getPixX()+cluster.getPixErrX(),
          cluster.getPixY()+cluster.getPixErrY(),
          iplane, ex, ey, ez);
      // And then store the difference to their nominal values
      cluster.setPosErr(std::fabs(ex-x), std::fabs(ey-y), std::fabs(ez-z));
    }
  }
}

}

