#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>

#include <Rtypes.h>

#include "mechanics/alignment.h"

namespace Mechanics {

class Sensor;

/**
  * Defines a device comprised of multiple sensors. The device has some 
  * attributes, as well as spatial information.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Device : public Alignment {
private:
  /** Number of sensors in this device, never changes */
  const size_t m_numSensors;
  /** The list of pointers to the device sensors */
  std::vector<Sensor*> m_sensors;

public:
  /** Name of this device, propagates to plots and results */
  std::string m_name;
  /** Conversion from clock tick number to time units */
  double m_clockRate;
  /** Duration of device read out, in units of clock ticks */
  unsigned m_readOutWindow;
  /** Name of the spatial units, propagates to plots */
  std::string m_spaceUnit;
  /** Name of the temporal units, propagates to plots */
  std::string m_timeUnit;

  /** Clock tick of first event in read out */
  ULong64_t m_timeStart;
  /** Clock tick of last event in read out */
  ULong64_t m_timeEnd;

  // TODO: copy constructor and sensors

  Device(size_t numSensors);
  ~Device();

  /** Print information about the device and sensors to cout */
  void print() const;
  /** Convert a clock tick to a time after the first read out */
  double tsToTime(ULong64_t timeStamp) const;

  /** Convert the pixel location of a sensor to global coodriates */
  void pixelToSpace(
      unsigned col,
      unsigned row,
      unsigned nsensor,
      double& x,
      double& y,
      double& z) const;

  /** Get the number of sensors in the device */
  inline size_t getNumSensors() const { return m_numSensors; } 
  /** Get a reference to the sensor `n` */
  inline Sensor& getSensor(size_t n) { return *m_sensors[n]; }
  /** Get a constant reference to the sensor `n`, so that constant devices
    * don't allow for mutation of their sensors */
  inline const Sensor& getSensorConst(size_t n) const { return *m_sensors[n]; }
  /** Shortcut for constant sensor reference */
  inline const Sensor& operator[](size_t n) const { return *m_sensors[n]; }
};

}

#endif // DEVICE_H

