#ifndef CLUSTERMAKER_H
#define CLUSTERMAKER_H

namespace Storage { class Hit; }
namespace Storage { class Cluster; }
namespace Storage { class Plane; }
namespace Storage { class Event; }

namespace Processors {

class ClusterMaker
{
private:
  const unsigned int _maxSeparationX;
  const unsigned int _maxSeparationY;
  const double _maxSeparation;

  void addNeighbours(const Storage::Hit* hit, const Storage::Plane* plane,
                     Storage::Cluster* cluster);
  void calculateCluster(Storage::Cluster* cluster);

public:
  ClusterMaker(unsigned int maxSeparationX, unsigned int maxSeparationY,
               double maxSeparation);

  void generateClusters(Storage::Event* event, unsigned int planeNum);
};

}

#endif // CLUSTERMAKER_H
