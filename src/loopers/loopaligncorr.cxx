#include <iostream>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include "storage/storagei.h"
#include "storage/event.h"
#include "mechanics/device.h"
#include "loopers/loopaligncorr.h"

namespace Loopers {

LoopAlignCorr::LoopAlignCorr(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs),
    m_devices(devices) {
  // Check that there is 1 input per device
  if (m_devices.size() != m_inputs.size())
    throw std::runtime_error("LoopAlignCorr::loop: device / inputs mismatch");
  // Check that the device sensors match the input planes
  for (size_t i = 0; i < m_devices.size(); i++)
    if (m_devices[i]->getNumSensors() != m_inputs[i]->getNumPlanes())
      throw std::runtime_error(
          "LoopAlignCorr::loop: device / input plane mismatch");
}

void LoopAlignCorr::loop() {
  // Sort all device planes by z-distance

  // Object holds plane information and can sort realtive to others by z
  struct PlaneZ {
    // Use smaller members for less allocation when sorting, c++11 move
    // semantics would be nice here
    unsigned iglobal;
    float z;
    PlaneZ() : iglobal(0), z(0) {}
    inline bool operator<(const PlaneZ& other) const { return z < other.z; }
  };

  // Fill a vector of plane objects, sortable by z
  std::vector<PlaneZ> planesz;
  size_t iglobal = 0;
  for (size_t i = 0; i < m_devices.size(); i++) {
    for (size_t j = 0; j < m_devices[i]->getNumSensors(); j++) {
      planesz.push_back(PlaneZ());
      planesz.back().iglobal = iglobal;
      planesz.back().z = m_devices[i]->getSensor(j).getOffZ();
      iglobal += 1;
    }
  }

  // Assign space in the plane map for all planes
  m_planeMap.assign(planesz.size(), 0);

  // Sort the objects (relatively light since the objects are just 2*64 bits),
  // but not time critical anyway
  std::sort(planesz.begin(), planesz.end());

  // The lowest z plane aligns relative to itself
  m_planeMap[planesz[0].iglobal] = planesz[0].iglobal;
  // The rest align relative to the previous plane in z
  for (size_t i = 1; i < planesz.size(); i++)  // i iterates sorted in z
    // Convert sorted i to a global indices
    m_planeMap[planesz[i].iglobal] = planesz[i-1].iglobal;

  Looper::loop();
}

void LoopAlignCorr::execute() {
  size_t iglobal = 0;
  for (size_t i = 0; i < m_devices.size(); i++) {
    const Storage::Event& event = *m_events[i];
    for (size_t j = 0; j < m_devices[i]->getNumSensors(); j++) {
      const Storage::Plane& plane = event.getPlane(j);
      
      iglobal += 1;
    }
  }
}

}

