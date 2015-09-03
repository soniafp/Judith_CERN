#ifndef DEVICE_H
#define DEVICE_H

#include <vector>
#include <math.h>
#include <string>

#include <Rtypes.h>

namespace Mechanics {

class Sensor;
class NoiseMask;
class Alignment;

class Device
{
private:
  //const char* _name;
  std::string _name;
  double _clockRate;
  unsigned int _readOutWindow;
  std::string _spaceUnit;
  std::string _timeUnit;
  double _beamSlopeX;
  double _beamSlopeY;
  ULong64_t _timeStart;
  ULong64_t _timeEnd;
  double _syncRatio;

  unsigned int _numSensors;
  std::vector<Sensor*> _sensors;
  std::vector<bool> _sensorMask;

  NoiseMask* _noiseMask;
  Alignment* _alignment;

public:
  Device(const char* name, const char* alignmentName = "", const char* noiseMaskName = "",
         double clockRate = 0, unsigned int readOutWindow = 0,
         const char* spaceUnit = "", const char* timeUnit = "");
  ~Device();

  void print() const;

  void addSensor(Sensor* sensor);
  void addMaskedSensor();
  double tsToTime(ULong64_t timeStamp) const;

  void setBeamSlopeX(double rotation);
  void setBeamSlopeY(double rotation);

  void setTimeStart(ULong64_t timeStamp);
  void setTimeEnd(ULong64_t timeStamp);

  void setSyncRatio(double ratio);

  unsigned int getNumSensors() const;
  Sensor* getSensor(unsigned int n) const;

  unsigned int getNumPixels() const;

  const std::vector<bool>* getSensorMask() const;
  const std::string getName() const;
  const char* getSpaceUnit() const;
  const char* getTimeUnit() const;
  double getClockRate() const;
  unsigned int getReadOutWindow() const;
  NoiseMask* getNoiseMask();
  Alignment* getAlignment();
  double getBeamSlopeX() const;
  double getBeamSlopeY() const;
  ULong64_t getTimeStart() const;
  ULong64_t getTimeEnd() const;
  double getSyncRatio() const;
};

}

#endif // DEVICE_H
