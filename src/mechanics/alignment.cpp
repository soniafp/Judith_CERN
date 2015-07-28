#include "alignment.h"

#include <cassert>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <float.h>
#include <iomanip>

#include "../configparser.h"
#include "device.h"
#include "sensor.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::fstream;
using std::cout;
using std::endl;

namespace Mechanics {

unsigned int Alignment::sensorFromHeader(std::string header)
{
  if (header.size() <= 7 || header.substr(0, 7).compare("Sensor "))
    throw "AlignmentFile: parsed an incorrect header";

  std::stringstream ss;
  ss << header.substr(7);

  unsigned int sensorNum = 0;
  ss >> sensorNum;

  if (sensorNum >= _device->getNumSensors())
    throw "AlignmentFile: sensor exceeds device range";

  return sensorNum;
}

void Alignment::writeFile()
{
  std::ofstream file;
  file.open(_fileName.c_str());

  if (!file.is_open())
    throw "AlginmentFile: unable to open file for writing";

  file << std::setprecision(9);
  file << std::fixed;

  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    Sensor* sensor = _device->getSensor(nsens);
    file << "[Sensor " << nsens << "]" << endl;
    file << "offset x   : " << sensor->getOffX() << endl;
    file << "offset y   : " << sensor->getOffY() << endl;
    file << "offset z   : " << sensor->getOffZ() << endl;
    file << "rotation x : " << sensor->getRotX() << endl;
    file << "rotation y : " << sensor->getRotY() << endl;
    file << "rotation z : " << sensor->getRotZ() << endl;
    file << "[End Sensor]\n" << endl;
  }

  file << "[Device]" << endl;
  file << "slope x    : " << _device->getBeamSlopeX() << endl;
  file << "slope y    : " << _device->getBeamSlopeY() << endl;
  if (_device->getSyncRatio() > 0)
    file << "sync ratio : " << _device->getSyncRatio() << endl;
  file << "[End Device]\n" << endl;

  file.close();
}

void Alignment::readFile()
{
  try
  {
    ConfigParser config(_fileName.c_str());

    for (unsigned int nrow = 0; nrow < config.getNumRows(); nrow++)
    {
      const ConfigParser::Row* row = config.getRow(nrow);

      // No action to take when encoutering a header
      if (row->isHeader) continue;

      if (!row->header.compare("Device"))
      {
        const double value = ConfigParser::valueToNumerical(row->value);
        if (!row->key.compare("slope x"))
          _device->setBeamSlopeX(value);
        else if (!row->key.compare("slope y"))
          _device->setBeamSlopeY(value);
        else if (!row->key.compare("sync ratio"))
          _device->setSyncRatio(value);
        else
          throw "Alignment: can't parse config row";

        continue;
      }

      if (!row->header.compare("End Sensor")) continue;

      unsigned int nsens = sensorFromHeader(row->header);
      Sensor* sensor = _device->getSensor(nsens);
      const double value = ConfigParser::valueToNumerical(row->value);
      if (!row->key.compare("offset x"))
        sensor->setOffX(value);
      else if (!row->key.compare("offset y"))
        sensor->setOffY(value);
      else if (!row->key.compare("offset z"))
        sensor->setOffZ(value);
      else if (!row->key.compare("rotation x"))
        sensor->setRotX(value);
      else if (!row->key.compare("rotation y"))
        sensor->setRotY(value);
      else if (!row->key.compare("rotation z"))
        sensor->setRotZ(value);
      else
        throw "Alignment: can't parse config row";
    }
  }
  catch (const char* e)
  {
    if (VERBOSE) cout << "WARNING :: Alignment file failed :: " << e << endl;
  }
}

const char* Alignment::getFileName() { return _fileName.c_str(); }

Alignment::Alignment(const char* fileName, Device* device) :
  _fileName(fileName), _device(device)
{
  assert(device && "Alignment: can't initialize with a null device");
}

}
