#ifndef EXAMPLEDUALANALYZER_H
#define EXAMPLEDUALANALYZER_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class ExampleDualAnalyzer : public DualAnalyzer
{
private:
   TH2D* _dummyHist;
   std::vector<TH1D*> _dummySensorHist;

   // Parameters used during event processing
   int _dummyParameter;

public:
  ExampleDualAnalyzer(const Mechanics::Device* refDevice,
                      const Mechanics::Device* dutDevice,
                      TDirectory* dir = 0,
                      const char* suffix = "",
                      /* Additional parameters with default values */
                      int dummyParameter = 0,
                      /* Histogram options */
                      unsigned int histBins = 100);

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();
};

}

#endif
