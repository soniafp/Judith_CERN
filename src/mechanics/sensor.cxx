#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cmath>

#include "mechanics/device.h"
#include "mechanics/sensor.h"

namespace Mechanics {

Sensor::Sensor() :
    Alignment(),
    m_device(0),
    m_name(),
    m_nrows(0),
    m_ncols(0),
    m_rowPitch(0),
    m_colPitch(0),
    m_xox0(0) {}

Sensor::Sensor(const Sensor& copy) :
    Alignment(copy),
    m_device(0),  // don't copy device ownership
    m_name(copy.m_name),
    m_nrows(copy.m_nrows),
    m_ncols(copy.m_ncols),
    m_rowPitch(copy.m_rowPitch),
    m_colPitch(copy.m_colPitch),
    m_xox0(copy.m_xox0) {}

void Sensor::print() const {
  std::printf(
      "---\nSensor: %s\n"
      "  rows:      %d\n"
      "  cols:      %d\n"
      "  row pitch: %8.2e\n"
      "  col pitch: %8.2e\n"
      "  device:    %s\n",
      m_name.c_str(),
      m_nrows,
      m_ncols,
      m_rowPitch,
      m_colPitch,
      (m_device ? m_device->m_name.c_str() : ""));
  std::cout << std::flush;
}

void Sensor::pixelToSpace(
    double col,
    double row,
    double& x,
    double& y,
    double& z) const {
  // Transform pixel space to sensor global space (in units of pitch). Location
  // of the pixel center, relative to the sensor center
  x = (col+0.5)*m_colPitch - m_ncols*m_colPitch/2.;
  y = (row+0.5)*m_rowPitch - m_nrows*m_rowPitch/2.;
  z = 0;
  // Then transform it into the device space (transformation of this plane
  // relative to the device). If no device exists, this is just global space.
  transform(x, y, z);
  // Then transform it into global space, if it belongs to a device.
  if (m_device) m_device->transform(x, y, z);
}

void Sensor::spaceToPixel(
    double x,
    double y,
    double z,
    double& col,
    double& row) const {
  // Remove the device transformations if appicable
  if (m_device) m_device->transform(x, y, z, true);
  // Remove sensor transformations relative to the device
  transform(x, y, z, true);
  // Now get the corresponding pixel unit coordinate
  col = x/m_colPitch + m_ncols/2. - 0.5;
  row = y/m_rowPitch + m_nrows/2. - 0.5;
}

unsigned Sensor::getPixelIndex(unsigned row, unsigned col) const { 
  return row*m_ncols + col;
}

bool Sensor::getPixelMask(unsigned row, unsigned col) const {
  if (m_noiseMask.empty()) throw std::runtime_error(
      "Sensor::getPixelMask: mask is not set");
  return m_noiseMask[getPixelIndex(row, col)];
}

bool Sensor::getPixelNoise(unsigned row, unsigned col) const {
  if (m_noiseProfile.empty()) throw std::runtime_error(
      "Sensor::getPixelMask: noise profile not set");
  return m_noiseProfile[getPixelIndex(row, col)];
}

void Sensor::getNormal(double& x, double& y, double& z) const {
  // Sensor origin in local coordinates
  double x1 = 0;
  double y1 = 0;
  double z1 = 0;
  // Get it in global coordinates
  transform(x1, y1, z1); 

  // Unit vector in locl coordinates
  double x2 = 0;
  double y2 = 0;
  double z2 = 1;
  transform(x2, y2, z2);

  // Get back the unit vector in global coordinates
  x = x2-x1;
  y = y2-y1;
  z = z2-z1;
}

void Sensor::getPixBox(double& x, double& y) const {
  double dummy = 0;

  // Middle of a pixel
  double x1 = 0;
  double y1 = 0;
  // In global coordinates
  pixelToSpace(0, 0, x1, y1, dummy);

  // Middle of neighbouring pixel
  double x2 = 0;
  double y2 = 0;
  pixelToSpace(1, 1, x2, y2, dummy);

  // Separation in X and Y of the two pixels
  x = std::fabs(x2-x1);
  y = std::fabs(y2-y1);
}

void Sensor::getSensorBox(double& x, double& y) const {
  double dummy = 0;

  // Bottom left pixel in local coordinates
  double x1 = 0;
  double y1 = 0;
  // In global coordinates
  pixelToSpace(-.5, -.5, x1, y1, dummy);

  // Top right pixel
  double x2 = 0;
  double y2 = 0;
  pixelToSpace(m_ncols-.5, m_nrows-.5, x2, y2, dummy);

  // Width and height
  x = std::fabs(x2-x1);
  y = std::fabs(y2-y1);
}

void Sensor::getSensorBox(
    double& x1,
    double& y1,
    double& x2,
    double& y2) const {
  double dummy = 0;
  pixelToSpace(-.5, -.5, x1, y1, dummy);
  pixelToSpace(m_ncols-.5, m_nrows-.5, x2, y2, dummy);
  if (x1 > x2) std::swap(x1, x2);
  if (y1 > y2) std::swap(y1, y2);
}

}

