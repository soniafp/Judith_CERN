#ifndef DUALANALYZER_H
#define DUALANALYZER_H

#include <vector>

#include <TDirectory.h>

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class EventCut;
class TrackCut;
class ClusterCut;
class HitCut;

class DualAnalyzer
{
// Everything is accessible to the derived class, but not externally
protected:
  const Mechanics::Device* _refDevice;
  const Mechanics::Device* _dutDevice;
  TDirectory* _dir;
  const char* _nameSuffix;
  bool _postProcessed;

  std::vector<const EventCut*> _eventCuts;
  unsigned int _numEventCuts;

  std::vector<const TrackCut*> _trackCuts;
  unsigned int _numTrackCuts;

  std::vector<const ClusterCut*> _clusterCuts;
  unsigned int _numClusterCuts;

  std::vector<const HitCut*> _hitCuts;
  unsigned int _numHitCuts;

  DualAnalyzer(const Mechanics::Device* refDevice,
               const Mechanics::Device* dutDevice,
               TDirectory* dir = 0,
               const char* nameSuffix = "");

  void validRefSensor(unsigned int nsensor);
  void validDutSensor(unsigned int nsensor);
  void eventDeivceAgree(const Storage::Event* refEvent,
                        const Storage::Event* dutEvent);
  TDirectory* makeGetDirectory(const char* dirName);


public:
  virtual ~DualAnalyzer();

  // This behaviour NEEDS to be implemented in the derived classes
  virtual void processEvent(const Storage::Event* refEvent,
                            const Storage::Event* dutEvent) = 0;
  virtual void postProcessing() = 0;

  void addCut(const EventCut* cut);
  void addCut(const TrackCut* cut);
  void addCut(const ClusterCut* cut);
  void addCut(const HitCut* cut);
};

}

#endif // DUALANALYZER_H
