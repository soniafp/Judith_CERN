#ifndef DEPICTION_h
#define DEPICTION_h

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }
namespace Processors { class EventDepictor; }

namespace Analyzers {

class Depictor : public SingleAnalyzer
{
private:
  const bool _depictEvent;
  const bool _depictClusters;
  const bool _depictTracks;

  Processors::EventDepictor* _depictor;

public:
  Depictor(const Mechanics::Device* refDevice,
           TDirectory* dir = 0,
           const char* suffix = "",
           /* Additional parameters with default values */
           bool depictEvent = true,
           bool depictClusters = true,
           bool depictTracks = true,
           double zoom = 20);

  void processEvent(const Storage::Event* refEvent);
  void postProcessing();
};

}

#endif
