#ifndef EVENTDEPICTOR_H
#define EVENTDEPICTOR_H

#include <vector>

namespace Storage { class Event; }
namespace Storage { class Track; }
namespace Storage { class Plane; }
namespace Storage { class Cluster; }
namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }

namespace Processors {

class EventDepictor
{
private:
  double _zoom;

  const Mechanics::Device* _refDevice;
  const Mechanics::Device* _dutDevice;

  void drawEventClusterHits(const Storage::Cluster* cluster);
  void drawEventCluster(const Storage::Cluster* cluster);
  void drawEventTrackIntercepts(const Storage::Track* track,
                           const Storage::Cluster* linkedCluster,
                           const Mechanics::Sensor* sensor);
  void depictEventSensor(const Storage::Plane* eventPlane,
                    const Mechanics::Sensor* sensor);

public:
  EventDepictor(const Mechanics::Device* refDevice,
                const Mechanics::Device* dutDevice = 0);

  void depictEvent(const Storage::Event* refEvent,
                   const Storage::Event* dutEvent = 0);

  void depictClusters(std::vector<const Storage::Cluster*>& refClusters,
                      std::vector<const Storage::Cluster*>& dutClusters);

  void depictTrack(const Storage::Track* track);

  void setZoom(double value) { _zoom = value; }
};

}

#endif // EVENTDEPICTOR_H
