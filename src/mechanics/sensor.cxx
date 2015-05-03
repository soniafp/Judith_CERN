#include <iostream>
#include <stdexcept>
#include <cstdio>

#include "mechanics/device.h"
#include "mechanics/sensor.h"

namespace Mechanics {

Sensor::Sensor() :
    m_name(),
    m_nrows(0),
    m_ncols(0),
    m_rowPitch(0),
    m_colPitch(0),
    m_xox0(0),
    m_device(0) {}

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
    double row,
    double col,
    double& x,
    double& y,
    double& z) const {
  x = (col+0.5)*m_colPitch - m_ncols*m_colPitch/2.;
  y = (row+0.5)*m_rowPitch - m_nrows*m_rowPitch/2.;
  z = 0;
  transform(x, y, z);
}

void Sensor::spaceToPixel(
    double x,
    double y,
    double z,
    double& row,
    double& col) const {
  transform(x, y, z, true);
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

}

