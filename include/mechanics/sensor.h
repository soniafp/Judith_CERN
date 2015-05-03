#ifndef SENSOR_H
#define SENSOR_H   

#include <string>
#include <vector>

#include "mechanics/alignment.h"

namespace Mechanics {

class Device;

class Sensor : public Alignment {
public:
  std::string m_name;
  unsigned m_nrows;
  unsigned m_ncols;
  double m_rowPitch;
  double m_colPitch;
  double m_xox0;

  Device* m_device;

  std::vector<double> m_noiseProfile;
  std::vector<bool> m_noiseMask;
  
  Sensor();
  ~Sensor() {}

  void print() const;

  void pixelToSpace(
      double row,
      double col,
      double& x,
      double& y,
      double& z) const;

  void spaceToPixel(
      double x,
      double y,
      double z,
      double& row,
      double& col) const;

  inline unsigned getPixelIndex(unsigned row, unsigned col) const;
  bool getPixelMask(unsigned row, unsigned col) const;
  bool getPixelNoise(unsigned row, unsigned col) const;
};

}

#endif  // SENSOR_H
