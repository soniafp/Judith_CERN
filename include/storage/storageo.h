#ifndef STORAGEO_H
#define STORAGEO_H

#include <string>
#include <vector>
#include <set>

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
      int treeMask = NONE,
      // List of branch names to turn off in the each tree
      const std::set<std::string>* hitsBranchesOff=0,
      const std::set<std::string>* clustersBranchesOff=0,
      const std::set<std::string>* tracksBranchesOff=0,
      const std::set<std::string>* eventInfoBranchesOff=0);
  // Write to the file
  virtual ~StorageO();

  /** Write the `Event` object to the file */
  void writeEvent(Event& event);
};

}

#endif // STORAGEO_H
