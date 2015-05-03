#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>

#include <Rtypes.h>

#include "mechanics/alignment.h"

namespace Mechanics {

class Sensor;

class Device : public Alignment {
private:
  const size_t m_numSensors;
  std::vector<Sensor*> m_sensors;

public:
  std::string m_name;
  double m_clockRate;
  unsigned m_readOutWindow;
  std::string m_spaceUnit;
  std::string m_timeUnit;

  ULong64_t m_timeStart;
  ULong64_t m_timeEnd;

  Device(size_t numSensors);
  ~Device();

  void print() const;
  double tsToTime(ULong64_t timeStamp) const;

  void pixelToSpace(
      unsigned col,
      unsigned row,
      unsigned nsensor,
      double& x,
      double& y,
      double& z) const;

  inline size_t getNumSensors() const { return m_numSensors; } 
  inline Sensor& getSensor(size_t n) const { return *m_sensors[n]; }
  inline Sensor& operator[](size_t n) const { return *m_sensors[n]; }
};

}

#endif // DEVICE_H

