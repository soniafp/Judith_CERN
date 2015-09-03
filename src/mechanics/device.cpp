#include "device.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <string.h>
#include <string>
#include <algorithm>

#include <Rtypes.h>

#include "sensor.h"
#include "noisemask.h"
#include "alignment.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

namespace Mechanics {

void Device::print() const
{
  cout << "\nDEVICE:\n"
       << "  Name: " << _name << "\n"
       << "  Clock rate: " << _clockRate << "\n"
       << "  Read out window: " << _readOutWindow << "\n"
       << "  Sensors: " << _numSensors << endl;

  for (unsigned int nsens = 0; nsens < getNumSensors(); nsens++)
    getSensor(nsens)->print();
}

void Device::addSensor(Sensor* sensor)
{
  assert(sensor && "Device: can't add a null sensor");
  if (_numSensors > 0 && getSensor(_numSensors - 1)->getOffZ() > sensor->getOffZ())
    throw "Device: sensors must be added in order of increazing Z position";
  _sensors.push_back(sensor);
  _sensorMask.push_back(false);
  _numSensors++;
}

void Device::addMaskedSensor() {
  _sensorMask.push_back(true);
}

double Device::tsToTime(ULong64_t timeStamp) const
{
  return (double)((timeStamp - getTimeStart()) / (double)getClockRate());
}

void Device::setBeamSlopeX(double rotation) { _beamSlopeX = rotation; }
void Device::setBeamSlopeY(double rotation) { _beamSlopeY = rotation; }

double Device::getBeamSlopeX() const { return _beamSlopeX; }
double Device::getBeamSlopeY() const { return _beamSlopeY; }

void Device::setTimeStart(ULong64_t timeStamp) { _timeStart = timeStamp; }
void Device::setTimeEnd(ULong64_t timeStamp) { _timeEnd = timeStamp; }

void Device::setSyncRatio(double ratio) { _syncRatio = ratio; }

ULong64_t Device::getTimeStart() const { return _timeStart; }
ULong64_t Device::getTimeEnd() const { return _timeEnd; }

unsigned int Device::getNumSensors() const { return _numSensors; }
Sensor* Device::getSensor(unsigned int n) const
{
  assert(n < _numSensors && "Device: sensor index outside range");
  return _sensors.at(n);
}

unsigned int Device::getNumPixels() const
{
  unsigned int numPixels = 0;
  for (unsigned int nsens = 0; nsens < getNumSensors(); nsens++)
    numPixels += getSensor(nsens)->getNumX() * getSensor(nsens)->getNumY();
  return numPixels;
}

const std::vector<bool>* Device::getSensorMask() const { return &_sensorMask; }
  //const char* Device::getName() const { return _name; }
  const std::string Device::getName() const { return _name; }  
const char* Device::getSpaceUnit() const { return _spaceUnit.c_str(); }
const char* Device::getTimeUnit() const { return _timeUnit.c_str(); }
double Device::getClockRate() const { return _clockRate; }
unsigned int Device::getReadOutWindow() const { return _readOutWindow; }
NoiseMask* Device::getNoiseMask() { return _noiseMask; }
Alignment* Device::getAlignment() { return _alignment; }
double Device::getSyncRatio() const { return _syncRatio; }

Device::Device(const char* name, const char* alignmentName, const char* noiseMaskName,
               double clockRate, unsigned int readOutWindow,
               const char* spaceUnit, const char* timeUnit) :
  _name(name), _clockRate(clockRate), _readOutWindow(readOutWindow),
  _spaceUnit(spaceUnit), _timeUnit(timeUnit),
  _beamSlopeX(0), _beamSlopeY(0),
  _timeStart(0), _timeEnd(0), _syncRatio(0),
  _numSensors(0), _noiseMask(0), _alignment(0)
{
  if (strlen(alignmentName))
    _alignment = new Alignment(alignmentName, this);

  if (strlen(noiseMaskName))
    _noiseMask =  new NoiseMask(noiseMaskName, this);

  std::replace(_timeUnit.begin(), _timeUnit.end(), '\\', '#');
  std::replace(_spaceUnit.begin(), _spaceUnit.end(), '\\', '#');
}

Device::~Device()
{
  for (unsigned int nsensor = 0; nsensor < _numSensors; nsensor++)
    delete _sensors.at(nsensor);
  if (_noiseMask) delete _noiseMask;
  if (_alignment) delete _alignment;
}

}
