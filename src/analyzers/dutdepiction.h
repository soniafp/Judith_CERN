#ifndef DUTDEPICTION_H
#define DUTDEPICTION_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }
namespace Processors { class EventDepictor; }

namespace Analyzers {

class DUTDepictor : public DualAnalyzer
{
private:
  const bool _depictEvent;
  const bool _depictClusters;
  const bool _depictTracks;

  Processors::EventDepictor* _depictor;

public:
  DUTDepictor(const Mechanics::Device* refDevice,
              const Mechanics::Device* dutDevice,
              TDirectory* dir = 0,
              const char* suffix = "",
              /* Additional parameters with default values */
              bool depictEvent = true,
              bool depictClusters = true,
              bool depictTracks = true,
              double zoom = 20);

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();
};

}

#endif
