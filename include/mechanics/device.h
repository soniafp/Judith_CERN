#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <string>

#include <Rtypes.h>

#include "mechanics/alignment.h"
#include "mechanics/sensor.h"

namespace Mechanics {

/**
  * Defines a device comprised of multiple sensors. The device has some 
  * attributes, as well as spatial information.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Device : public Alignment {
private:
  /** The list of all sensors in this device (including masked) */
  std::vector<Sensor> m_sensorsFull;
  /** The list of pointers to the active sensors */
  std::vector<Sensor*> m_sensors;
  /** Remember which sensors were masked */
  std::vector<bool> m_sensorMask;

  /** No assignment */
  Device& operator=(const Device&);
  /** Update the list of active sensors from the mask */
  void updateSensors();

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
  /** Path of file where this device can store alignment information */
  std::string m_alignmentFile;

  /** Clock tick of first event in read out */
  ULong64_t m_timeStart;
  /** Clock tick of last event in read out */
  ULong64_t m_timeEnd;

  Device(size_t numSensors);
  Device(const Device& copy);
  ~Device() {}

  /** Print information about the device and sensors to cout */
  void print() const;
  /** Convert a clock tick to a time after the first read out */
  double tsToTime(ULong64_t timeStamp) const;

  /** Convert the pixel location of a sensor to global coodriates */
  void pixelToSpace(
      double col,
      double row,
      unsigned nsensor,
      double& x,
      double& y,
      double& z) const;

  /** Mask the sensor at index `n`, from the list of all sensors */
  void maskSensor(size_t n, bool mask=true);
  /** Get the mask applied to the full list of sensors */
  const std::vector<bool>& getSensorMask() const { return m_sensorMask; }
  /** Get the number of sensors in the device */
  inline size_t getNumSensors() const { return m_sensors.size(); } 
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

