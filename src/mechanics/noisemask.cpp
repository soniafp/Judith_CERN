#include "noisemask.h"

#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "device.h"
#include "sensor.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::fstream;
using std::cout;
using std::endl;

namespace Mechanics {

void NoiseMask::parseLine(std::stringstream& line, unsigned int& nsens,
                          unsigned int& x, unsigned int& y)
{
  unsigned int counter = 0;
  while (line.good())
  {
    std::string value;
    std::getline(line, value, ',');
    std::stringstream convert(value);
    switch (counter)
    {
    case 0:
      convert >> nsens;
      break;
    case 1:
      convert >> x;
      break;
    case 2:
      convert >> y;
      break;
    }
    counter++;
  }

  if (counter != 3) throw "NoiseMask: bad line found in file";
}

void NoiseMask::writeMask()
{
  std::fstream file;
  file.open(_fileName.c_str(), std::ios_base::out);

  if (!file.is_open())
    throw "NosieMask: unable to open file for writing";

  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
  {
    Sensor* sensor = _device->getSensor(nsens);
    for (unsigned int x = 0; x < sensor->getNumX(); x++)
      for (unsigned int y = 0; y < sensor->getNumY(); y++)
        if (sensor->isPixelNoisy(x, y))
          file << nsens << ", " << x << ", " << y << endl;
  }

  file.close();
}

void NoiseMask::readMask()
{
  std::fstream file;
  file.open(_fileName.c_str(), std::ios_base::in);

  if (!file.is_open())
  {
    if (VERBOSE) cout << "WARNING :: Noise mask failed to open file" << endl;
    return;
  }

  while (file.good())
  {
    unsigned int nsens = 0;
    unsigned int x = 0;
    unsigned int y = 0;
    std::string line;
    std::getline(file, line);
    if (!line.size()) continue;
    std::stringstream lineStream(line);
    parseLine(lineStream, nsens, x, y);
    if (nsens >= _device->getNumSensors())
      throw "NoiseMask: specified sensor outside device range";
    if (x >= _device->getSensor(nsens)->getNumX())
      throw "NoiseMask: specified x position outside sensor range";
    if (y >= _device->getSensor(nsens)->getNumY())
      throw "NoiseMask: specified y position outside sensor range";
    _device->getSensor(nsens)->addNoisyPixel(x, y);
  }

  file.close();
}

std::vector<bool**> NoiseMask::getMaskArrays() const {
  std::vector<bool**> masks;
  for (unsigned int nsens = 0; nsens < _device->getNumSensors(); nsens++)
    masks.push_back(_device->getSensor(nsens)->getNoiseMask());
  return masks;
}

const char* NoiseMask::getFileName() { return _fileName.c_str(); }

NoiseMask::NoiseMask(const char* fileName, Device* device) :
  _fileName(fileName), _device(device)
{
  assert(device && "NoiseMask can't initialize with a null device");
}

}
