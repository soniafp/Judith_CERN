#ifndef PARSEDEVICE_H
#define PARSEDEVICE_H

#include <string>

namespace Mechanics {

class Device;

Device* parseDevice(const std::string& filePath);

}

#endif  // PARSEDEVICE_H

