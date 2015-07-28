#ifndef EVENTINFO_H
#define EVENTINFO_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class EventInfo : public SingleAnalyzer
{
private:
  TH1D* _triggerOffset;
  TH1D* _trackInTime;
  TH1D* _numTracks;
  TH1D* _eventsVsTime;
  TH1D* _tracksVsTime;
  TH1D* _clustersVsTime;

public:
  EventInfo(const Mechanics::Device* device,
            TDirectory* dir,
            const char* suffix = "",
            /* Histogram options */
            unsigned int maxTracks = 10);

  void processEvent(const Storage::Event* event);
  void postProcessing();
};

}

#endif
