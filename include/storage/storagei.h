#ifndef STORAGEI_H
#define STORAGEI_H

#include <string>
#include <vector>
#include <set>

#include "storage/storageio.h"

namespace Storage {

class StorageI : public StorageIO {
private:
  // Disable copy and assignment operators
  StorageI(const StorageI&);
  StorageI& operator=(const StorageI&);

public:
  StorageI(
      const std::string& filePath,
      int treeMask=NONE,
      const std::vector<bool>* planeMask=0,
      // List of branch names to turn off in the each tree
      const std::set<std::string>* hitsBranchesOff=0,
      const std::set<std::string>* clustersBranchesOff=0,
      const std::set<std::string>* tracksBranchesOff=0,
      const std::set<std::string>* eventInfoBranchesOff=0);
  virtual ~StorageI() {}

  /** Generate the `Event` object filled from entry `n` */
  Event& readEvent(Long64_t n);
};

}

#endif // STORAGEI_H
