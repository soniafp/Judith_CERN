#ifndef STORAGEO_H
#define STORAGEO_H

#include <string>
#include <vector>

#include "storage/storageio.h"

namespace Storage {

class StorageO : public StorageIO {
private:
  // Disable copy and assignment operators
  StorageO(const StorageIO&);
  StorageO& operator=(const StorageIO&);

public:
  StorageO(
      const std::string& filePath,
      size_t numPlanes,
      int contentMask = NONE);
  // Write to the file
  virtual ~StorageO();

  /** Write the `Event` object to the file */
  void writeEvent(Event& event);
};

}

#endif // STORAGEO_H
