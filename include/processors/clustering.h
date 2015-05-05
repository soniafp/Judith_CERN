#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <list>

namespace Storage { class Hit; }
namespace Storage { class Cluster; }
namespace Storage { class Event; }

namespace Processors {

class Clustering {
private:
  unsigned m_maxRows;
  unsigned m_maxCols;

  virtual void clusterSeed(
      Storage::Hit& seed,
      std::list<Storage::Hit*>& hits,
      std::list<Storage::Hit*>& clustered);
  virtual void buildCluster(
      Storage::Cluster& cluster,
      std::list<Storage::Hit*>& clustered);

public:
  Clustering();
  virtual ~Clustering() {}

  virtual void process(Storage::Event& event);
  virtual void setMaxPixels(unsigned cols, unsigned rows);
};

}

#endif  // CLUSTERING_H
