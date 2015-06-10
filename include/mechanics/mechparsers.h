#ifndef MECHPARSERS_H
#define MECHPARSERS_H

#include <string>
#include <map>

namespace Mechanics {

class Device;

/** Build a device from the information given in a file */
Device* parseDevice(const std::string& filePath);

/** Set a device's alignment based on its alignment file. */
void parseAlignment(Device& device);

/** Write out a device's alignment */
void writeAlignment(Device& device);

/**
  * Container of devices parsed. Useful when setting up a multi-device run.
  * Provies nammed and indexed access to the devices.
  *
  * @author Garrin McGoldrick
  */
class Devices {
private: 
  /** List of devices parsed */
  std::vector<Mechanics::Device*> devices;
  /* Map of device name to its object */
  std::map<std::string, Mechanics::Device*> map;

public:
  Devices() {}
  ~Devices();

  /** Given a device pointer (e.g. from parseDevice), take ownership of it */
  void addDevice(Mechanics::Device* device);
  /** Access device by index using the [] operator */
  inline Mechanics::Device& operator[](size_t n) { return *devices[n]; }
  /** Access device by name using the [] operator */
  inline Mechanics::Device& operator[](const std::string& name) { return *map[name]; }
  inline size_t getNumDevices() const { return devices.size(); }
  /** Reference to the device vector */
  inline const std::vector<Mechanics::Device*>& getVector() const { return devices; }
};

}

#endif  // MECHPARSERS_H

