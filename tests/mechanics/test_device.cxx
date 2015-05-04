#include <iostream>
#include <stdexcept>
#include <cmath>

#include "mechanics/alignment.h"
#include "mechanics/sensor.h"
#include "mechanics/device.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_sensors() {
  Mechanics::Device device(2);

  if (device.getNumSensors() != 2) {
    std::cerr << "Sensor allocation failed" << std::endl;
    return -1;
  }

  if (&device[0] == 0 || &device[1] == 0 || &device[0] == &device[1]) {
    std::cerr << "Sensor access/allocation failed" << std::endl;
    return -1;
  }

  if (device[0].getDevice() != &device || device[1].getDevice() != &device) {
    std::cerr << "Sensor device association failed" << std::endl;
    return -1;
  }

  return 0;
}

int test_pixelToSpace() {
  // NOTE: most of this functionality is tested in test_alignment and
  // test_sensor. This is just checking that the device alignment is proagating
  // and is also serving as a sanity check.

  Mechanics::Device device(2);
  device.setOffX(.1);
  device.setOffY(-.1);
  device.setOffZ(.2);
  device.setRotY(0.3);

  Mechanics::Sensor& sensor1 = device.getSensor(1);

  sensor1.m_nrows = 2;
  sensor1.m_ncols = 2;
  sensor1.m_rowPitch = 1;
  sensor1.m_colPitch = 1;
  sensor1.setOffX(1);
  sensor1.setOffY(-1);
  sensor1.setRotZ(-0.5);

  Mechanics::Alignment local;
  local.setRotZ(-0.5);
  Mechanics::Alignment global;
  global.setRotY(0.3);

  // Coodrinates of 0,1 on the sensor, in pixel space
  double x0 = -0.5;
  double y0 = 0.5;
  double z0 = 0;
  // Apply the local rotation
  local.transform(x0, y0, z0);
  // Apply the local offset
  x0 += 1;
  y0 += -1;
  // Apply the global rotation
  global.transform(x0, y0, z0);
  // Apply the global offset
  x0 += .1;
  y0 += -.1;
  z0 += .2;

  // Finally, check if this compares to the overal transformation of pixel
  // 0,1 in sensor 1
  double x, y, z;
  device.pixelToSpace(0, 1, 1, x, y, z);

  if (!approxEqual(x, x0) || !approxEqual(y, y0) || !approxEqual(z, z0)) {
    std::cerr << "pixelToSpace failed to apply device transformation" << std::endl;
    return -1;
  }

  return 0;
}

int test_copy() {
  Mechanics::Device device(2);
  Mechanics::Device copy(device);

  if (copy.getNumSensors() != 2) {
    std::cerr << "Copy sensor allocation failed" << std::endl;
    return -1;
  }

  if (&copy[0] == 0 || &copy[1] == 0 ||
      &copy[0] == &device[0] || &copy[1] == &device[1]) {
    std::cerr << "Copy sensor access/allocation failed" << std::endl;
    return -1;
  }

  if (copy[0].getDevice() != &copy || copy[1].getDevice() != &copy) {
    std::cerr << "Copy sensor association failed" << std::endl;
    return -1;
  }

  return 0;
}

int test_masking() {
  Mechanics::Device device(2);
  device.getSensor(0).m_name = "Sensor0";
  device.getSensor(1).m_name = "Sensor1";

  device.maskSensor(0);

  if (device.getNumSensors() != 1 || device[0].m_name != "Sensor1") {
    std::cerr << "Sensor masking failed" << std::endl;
    return -1;
  }

  device.maskSensor(0, false);

  if (device.getNumSensors() != 2 ||
      device[0].m_name != "Sensor0" || device[1].m_name != "Sensor1") {
    std::cerr << "Sensor un-masking failed" << std::endl;
    return -1;
  }

  device.maskSensor(1);

  if (device.getNumSensors() != 1 || device[0].m_name != "Sensor0") {
    std::cerr << "Sensor re-masking failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_sensors()) != 0) return retval;
    if ((retval = test_pixelToSpace()) != 0) return retval;
    if ((retval = test_copy()) != 0) return retval;
    if ((retval = test_masking()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
