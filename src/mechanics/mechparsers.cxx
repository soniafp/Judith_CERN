#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "mechanics/sensor.h"
#include "mechanics/device.h"
#include "mechanics/mechparsers.h"

namespace Mechanics {

template <class T>
inline void strToVal(const std::string& str, T& val) {
  std::stringstream ss(str);
  ss >> val;
}

inline double strToFloat(const std::string& str) {
  double val = 0;
  strToVal(str, val);
  return val;
}

inline int strToInt(const std::string& str) {
  int val = 0;
  strToVal(str, val);
  return val;
}

void prepareLine(std::string& line) {
  const size_t start = line.find_first_not_of(" \t\r\n");
  // Keep everything up to a # if there is one
  const size_t clip = line.find('#');
  if (start == std::string::npos)
    line = "";
  else
    line = line.substr(start, (clip==std::string::npos) ? clip : clip-start);
}

void trim(std::string& s) {
  const size_t start = s.find_first_not_of(" \t\r\n");
  const size_t end = s.find_last_not_of(" \t\r\n");
  if (start == std::string::npos)
    s = "";
  else
    s = s.substr(start, end+1-start);
}

// Collection of spatial information accumulated while pasring
struct SpatialBuff {
  double offX;
  double offY;
  double offZ;
  double rotX;
  double rotY;
  double rotZ;
  SpatialBuff() : offX(0), offY(0), offZ(0), rotX(0), rotY(0), rotZ(0) {}
};

// Collection of device values while parsing
struct DeviceBuff : public SpatialBuff {
  std::string name;
  std::string spaceUnit;
  std::string timeUnit;
  std::string alignmentFile;
  double clock;
  int window;
  DeviceBuff() : clock(0), window(0) {}
};

// Collection of sensor values while parsing
struct SensorBuff : public SpatialBuff {
  std::string name;
  std::string chip;
  SensorBuff() {}
};

// Collection of chip values while parsing
struct ChipBuff {
  std::string name;
  int rows;
  int cols;
  double rowPitch;
  double colPitch;
  ChipBuff() : rows(0), cols(0), rowPitch(0), colPitch(0) {}
};

bool sortSensors(const SensorBuff* lhs, const SensorBuff* rhs) {
  return lhs->offZ <= rhs->offZ;
}

// Try to assign a key, value pair to a spatial buffer.
bool spatialKey(
    const std::string& key,
    const std::string& value,
    SpatialBuff& buff) {
  if (key == "off-x") buff.offX = strToFloat(value);
  else if (key == "off-y") buff.offY = strToFloat(value);
  else if (key == "off-z") buff.offZ = strToFloat(value);
  else if (key == "rot-x") buff.rotX = strToFloat(value);
  else if (key == "rot-y") buff.rotY = strToFloat(value);
  else if (key == "rot-z") buff.rotZ = strToFloat(value);
  else return false;  // no spatial key found
  return true;  // spatial key was found
}

// Set the spatial alignment values in an alignment derived object from those
// values stored in a buffer
void setBuffAlignment(
    Alignment& object,
    const SpatialBuff& buffer) {
  object.setOffX(buffer.offX);
  object.setOffY(buffer.offY);
  object.setOffZ(buffer.offZ);
  object.setRotX(buffer.rotX);
  object.setRotY(buffer.rotY);
  object.setRotZ(buffer.rotZ);
}

Device* parseDevice(const std::string& filePath) {
  // The device being generated
  DeviceBuff device;
  // The sensors collected
  std::vector<SensorBuff> sensors;
  // The chips collected
  std::vector<ChipBuff> chips;

  // Keep track of the current object being parsed
  enum Current { NONE, DEVICE, SENSOR, CHIP };
  Current current = NONE;

  std::ifstream file(filePath.c_str());
  if (!file) throw "Mechanics: parseDevice: unable to open file";

  std::string line;
  // Read each line in the file
  while (std::getline(file, line)) {
    prepareLine(line);  // crop to #, and trim leading space
    if (line.empty()) continue;

    const size_t split = line.find(' ');
    // Get the first word
    std::string key = line.substr(0, split);
    // Get everything after the word
    std::string value = (split==std::string::npos) ? "" : line.substr(split);
    // Trim leading and trailing white space
    trim(value);


    if (key == "device:") {
      current = DEVICE;
      device.name = value;
    }

    else if (key == "sensor:") {
      current = SENSOR;
      sensors.push_back(SensorBuff());
      sensors.back().name = value;
    }
    
    else if (key == "chip:") {
      current = CHIP;
      chips.push_back(ChipBuff());
      chips.back().name = value;
    }

    else if (current == DEVICE) {
      if (key == "space-unit") device.spaceUnit = value;
      else if (key == "time-unit") device.timeUnit = value;
      else if (key == "clock") device.clock = strToFloat(value);
      else if (key == "window") device.window = strToInt(value);
      else if (key == "alignment") device.alignmentFile = value;
      else if (spatialKey(key, value, device)) continue;
      else throw std::runtime_error("Mechanics: parseDevice: unknown device key");
    }

    else if (current == SENSOR) {
      SensorBuff& sensor = sensors.back();
      if (key == "chip") sensor.chip = value;
      else if (spatialKey(key, value, sensor)) continue;
      else throw std::runtime_error("Mechanics: parseDevice: unknown sensor key");
    }

    else if (current == CHIP) {
      ChipBuff& chip = chips.back();
      if (key == "rows") chip.rows = strToInt(value);
      else if (key == "cols") chip.cols = strToInt(value);
      else if (key == "row-pitch") chip.rowPitch = strToFloat(value);
      else if (key == "col-pitch") chip.colPitch = strToFloat(value);
      else throw std::runtime_error("Mechanics: parseDevice: unknown chip key");
    }
  }
  file.close();

  // Build a mapping of chip name to chip object
  std::map<std::string, ChipBuff*> chipMap;
  for (std::vector<ChipBuff>::iterator it = chips.begin();
      it != chips.end(); ++it) {
    ChipBuff& chip = *it;
    if (chip.name.empty())
      throw std::runtime_error("Mechanics: parseDevice: chip has no name");
    if (chipMap.find(chip.name) != chipMap.end())
      throw std::runtime_error("Mechanics: parseDevice: duplicate chip name");
    chipMap[chip.name] = &chip;
  }

  // Sort the sensors
  std::vector<SensorBuff*> sorted(sensors.size());
  for (size_t i = 0; i < sensors.size(); i++)
    sorted[i] = &sensors[i];
  std::sort(sorted.begin(), sorted.end(), sortSensors);

  // Name the sensors as necessary, and verify validity of chips
  for (size_t i = 0; i < sorted.size(); i++) {
    SensorBuff& sensor = *sorted[i];
    if (chipMap.find(sensor.chip) == chipMap.end())
      throw std::runtime_error("Mechanics: parseDevice: invalid sensor chip name");
    if (sensor.name.empty()) {
      std::stringstream ss;
      ss << "Sensor" << i;
      sensor.name = ss.str();
    }
  }

  if (device.name.empty())
    throw std::runtime_error("Mechanics: parseDevice: no device name given");

  // Build the device
  Device* deviceObj = new Device(sensors.size());
  deviceObj->m_name = device.name;
  deviceObj->m_clockRate = device.clock;
  deviceObj->m_readOutWindow = device.window;
  deviceObj->m_spaceUnit = device.spaceUnit;
  deviceObj->m_timeUnit = device.timeUnit;
  deviceObj->m_alignmentFile = device.alignmentFile;
  // Set the spatial alignment
  setBuffAlignment(*deviceObj, device);

  // Configure the sensors
  for (size_t i = 0; i < deviceObj->getNumSensors(); i++) {
    Sensor& sensorObj = deviceObj->getSensor(i);
    SensorBuff& sensor = *sorted[i];
    ChipBuff& chip = *chipMap[sensor.chip];
    sensorObj.m_name = sensor.name;
    sensorObj.m_nrows = chip.rows;
    sensorObj.m_ncols = chip.cols;
    sensorObj.m_rowPitch = chip.rowPitch;
    sensorObj.m_colPitch = chip.colPitch;
    // Set the spatial alignment
    setBuffAlignment(sensorObj, sensor);
  }

  // If an alignment file is specified and exists, parse and apply it
  if (!deviceObj->m_alignmentFile.empty()) {
    std::ifstream test(deviceObj->m_alignmentFile.c_str());
    const bool pass = test.is_open();
    test.close();
    if (pass) parseAlignment(*deviceObj);
  }

  return deviceObj;
}

void parseAlignment(Device& device) {
  std::ifstream file(device.m_alignmentFile.c_str());
  if (!file) throw "Mechanics: parsealignment: unable to open file";

  // Map sensor names to their objects
  std::map<std::string, Sensor*> sensorMap;
  for (size_t i = 0; i < device.getNumSensors(); i++)
    sensorMap[device.getSensor(i).m_name] = &device.getSensor(i);

  // The object being aligned (device or sensor)
  Alignment* object = 0;

  std::string line;
  // Read each line in the file
  while (std::getline(file, line)) {
    prepareLine(line);  // crop to #, and trim leading space
    if (line.empty()) continue;

    const size_t split = line.find(' ');
    // Get the first word
    std::string key = line.substr(0, split);
    // Get everything after the word
    std::string value = (split==std::string::npos) ? "" : line.substr(split);
    // Trim leading and trailing white space
    trim(value);

    if (key == "sensor:") {
      if (sensorMap.find(value) == sensorMap.end())
        throw std::runtime_error("Mechanics: parseAlignment: sensor not found");
      object = sensorMap[value];
      continue;
    }
    
    else if (key == "device:") {
      object = &device;
      continue;
    }

    if (!object) throw std::runtime_error(
          "Mechanics: parseAligment: no object nammed for alignment");

    if (key == "off-x") object->setOffX(strToFloat(value));
    else if (key == "off-y") object->setOffY(strToFloat(value));
    else if (key == "off-z") object->setOffZ(strToFloat(value));
    else if (key == "rot-x") object->setRotX(strToFloat(value));
    else if (key == "rot-y") object->setRotY(strToFloat(value));
    else if (key == "rot-z") object->setRotZ(strToFloat(value));
  }
  file.close();
}

// Write the alignment of a given alignment object to an out stream
void writeAlignment(const Alignment& object, std::ofstream& out) {
  out << "off-x " << object.getOffX() << "\n"
      << "off-y " << object.getOffY() << "\n"
      << "off-z " << object.getOffZ() << "\n"
      << "rot-x " << object.getRotX() << "\n"
      << "rot-y " << object.getRotY() << "\n"
      << "rot-z " << object.getRotZ() << std::endl;
}

void writeAlignment(Device& device) {
  // Open the devices' alignment fi;e
  std::ofstream file(device.m_alignmentFile.c_str());
  if (!file) throw "Mechanics: writeAlignment: unable to open file";

  // Ensure the values are written with an appropriate precision
  file << std::scientific;
  file.precision(6);

  // Write the device alignment
  file << "device: " << device.m_name << std::endl;
  writeAlignment(device, file);
  file << std::endl;

  // Write each sensor alignment
  for (size_t i = 0; i < device.getNumSensors(); i++) {
    file << "sensor: " << device[i].m_name << std::endl;
    writeAlignment(device[i], file);
    file << std::endl;
  }

  file.close();
}

Devices::~Devices() {
  for (std::vector<Mechanics::Device*>::iterator it = devices.begin();
      it != devices.end(); ++it)
    if (*it) delete *it;
}

void Devices::addDevice(Mechanics::Device* device) {
  if (!device)
    throw std::runtime_error("Devices::addDevice: null device given");
  // Take ownership right away so that its memory is now managed
  devices.push_back(device);
  if (device->m_name.empty())
    throw std::runtime_error("Devices::addDevice: device must be nammed");
  if (map.find(device->m_name) != map.end())
    throw std::runtime_error("Devices::addDevice: duplicate device name");
  map[device->m_name] = device;
}

}

