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
      size_t numPlanes,
      const std::string& filePath,
      int treeMask = NONE,
      const std::vector<bool>& planeMask=s_dummyMask);
  virtual ~StorageO() {}

  /** Write the `Event` object to the file */
  void writeEvent(Event& event);
};

}

#endif // STORAGEO_H
