#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstdio>

#include <Rtypes.h>

#include "mechanics/alignment.h"
#include "mechanics/sensor.h"
#include "mechanics/device.h"

namespace Mechanics {

Device::Device(size_t numSensors) :
    Alignment(),
    m_sensorsFull(numSensors),  // builds the sensor objects
    m_sensors(),  // filled when updating sensors
    m_sensorMask(numSensors, false),  // default to all active
    m_name(),
    m_clockRate(0),
    m_readOutWindow(0),
    m_spaceUnit(),
    m_timeUnit(),
    m_alignmentFile(),
    m_timeStart(0),
    m_timeEnd(0) {
  updateSensors();
}

Device::Device(const Device& copy) :
    Alignment(),
    m_sensorsFull(copy.m_sensorsFull),  // copies the sensor objects
    m_sensors(),  // don't copy the sensor pointer list
    m_sensorMask(copy.m_sensorMask),
    m_name(copy.m_name),
    m_clockRate(copy.m_clockRate),
    m_readOutWindow(copy.m_readOutWindow),
    m_spaceUnit(copy.m_spaceUnit),
    m_timeUnit(copy.m_timeUnit),
    m_timeStart(copy.m_timeStart),
    m_timeEnd(copy.m_timeEnd) {
  updateSensors();
}

void Device::updateSensors() {
  // Build up the active sensor list from the mask and full lists
  m_sensors.clear();
  m_sensors.reserve(m_sensorsFull.size());
  for (size_t i = 0; i < m_sensorsFull.size(); i++) {
    // If the sensor isn't masked, add it to the list of active sensors
    if (!m_sensorMask[i]) m_sensors.push_back(&m_sensorsFull[i]);
    // Associate the sensors to this device (useful when copying)
    m_sensorsFull[i].m_device = this;
  }
}

void Device::print() const {
  std::printf(
      "\n---\nDevice: %s\n---\n"
      "  clock rate:      %8.2e\n"
      "  read out window: %d\n",
      m_name.c_str(),
      m_clockRate,
      m_readOutWindow);
  for (size_t i = 0; i < getNumSensors(); i++)
    m_sensors[i]->print();
  std::cout << std::flush;
}

double Device::tsToTime(ULong64_t timeStamp) const {
  return (timeStamp-m_timeStart) / (double)m_clockRate;
}

void Device::pixelToSpace(
    double col,
    double row,
    unsigned nsensor,
    double& x,
    double& y,
    double& z) const {
  // pixelToSpace of the sensor will automatically call this device's transform
  // to go to global coodrinates, so just call that method.
  m_sensors[nsensor]->pixelToSpace(col, row, x, y, z);
}

void Device::maskSensor(size_t n, bool mask) {
  m_sensorMask[n] = mask;
  // Not the fastest way to do this, but this is far from speed critical
  updateSensors();
}

}
