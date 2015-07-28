#ifndef ALIGNMENT_H
#define ALIGNMENT_H

#include <string>

namespace Mechanics {

class Device;

class Alignment
{
private:
  const std::string _fileName;
  Device* _device;

  unsigned int sensorFromHeader(std::string header);

public:
  Alignment(const char* fileName, Device* device);

  void readFile();
  void writeFile();

  const char* getFileName();
};

}

#endif // ALIGNMENT_H
