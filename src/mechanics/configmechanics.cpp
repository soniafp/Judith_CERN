#include "configmechanics.h"

#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <string.h>

#include "../configparser.h"
#include "device.h"
#include "sensor.h"

namespace Mechanics {

Device* generateDevice(const ConfigParser& config)
{
  Device* device = 0;
  std::string name = "";
  std::string alignmentName = "";
  std::string noiseMaskName = "";
  double clockRate = 0;
  double readOutWindow = 0;
  std::string spaceUnit = "";
  std::string timeUnit = "";

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);
      
    if (row->isHeader && !row->header.compare("End Device"))
    {
      std::cout <<"device: "<< name.c_str() << std::endl;
      device = new Device(name.c_str(), alignmentName.c_str(),
                          noiseMaskName.c_str(), clockRate,
                          readOutWindow, spaceUnit.c_str(), timeUnit.c_str());

      generateSensors(config, device);

      return device;
    }

    if (row->isHeader)
      continue;

    if (row->header.compare("Device"))
      continue; // Skip non-device rows

    if (!row->key.compare("name"))
      name = row->value;
    else if (!row->key.compare("alignment"))
      alignmentName = row->value;
    else if (!row->key.compare("noise mask"))
      noiseMaskName = row->value;
    else if (!row->key.compare("clock"))
      clockRate = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("window"))
      readOutWindow = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("space unit"))
      spaceUnit = row->value;
    else if (!row->key.compare("time unit"))
      timeUnit = row->value;
    else
      throw "Mechanics: can't parse device row";

    std::cout << "loaded name: "<< row->value << std::endl;
  }

  // Control shouldn't arrive at this point
  throw "Mechanics: no device was parsed";
}

void generateSensors(const ConfigParser& config, Device* device)
{
  assert(device && "Mechanics: can't generate sensors with no devices present");

  double offX = 0;
  double offY = 0;
  double offZ = 0;
  double rotX = 0;
  double rotY = 0;
  double rotZ = 0;
  unsigned int cols = 0;
  unsigned int rows = 0;
  double depth = 0;
  double pitchX = 0;
  double pitchY = 0;
  double xox0 = 0;
  std::string name = "";
  bool masked = false;

  unsigned int sensorCounter = 0;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Sensor"))
    {
      if (!masked)
      //if ()
      {
        if (cols <= 0 || rows <= 0 || pitchX <= 0 || pitchY <= 0)
          throw "Mechanics: need cols, rows, pitchX and pitchY to make a sensor";

        if (!name.size())
        {
          std::stringstream ss;
          ss << "Plane" << sensorCounter;
          name = ss.str();
        }
        Sensor* sensor = new Sensor(cols, rows, pitchX, pitchY, depth, device, name,
                                    xox0, offX, offY, offZ, rotX, rotY, rotZ);
        device->addSensor(sensor);
        sensorCounter++;
      } 
      else
      {
        device->addMaskedSensor(); 
      }

      offX = 0;
      offY = 0;
      offZ = 0;
      rotX = 0;
      rotY = 0;
      rotZ = 0;
      cols = 0;
      rows = 0;
      depth = 0;
      pitchX = 0;
      pitchY = 0;
      xox0 = 0;
      name = "";
      masked = false;

      continue;
    }

    if (row->isHeader)
      continue;

    if (row->header.compare("Sensor"))
      continue;

    if (!row->key.compare("offset x"))
      offX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("offset y"))
      offY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("offset z"))
      offZ = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("rotation x"))
      rotX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("rotation y"))
      rotY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("rotation z"))
      rotZ = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("cols"))
      cols = (int)ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("rows"))
      rows = (int)ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("depth"))
      depth = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pitch x"))
      pitchX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pitch y"))
      pitchY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("xox0"))
      xox0 = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("name"))
      name = row->value;
    else if (!row->key.compare("masked"))
      masked = ConfigParser::valueToLogical(row->value);
    else
      throw "Mechanics: can't parse sensor row";
  }
}

}
