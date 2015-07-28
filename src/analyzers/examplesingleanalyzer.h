#ifndef EXAMPLESINGLEANALYZER_H
#define EXAMPLESINGLEANALYZER_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class ExampleSingleAnalyzer : public SingleAnalyzer
{
private:
   TH2D* _dummyHist;
   std::vector<TH1D*> _dummySensorHist;

   // Parameters used during event processing
   int _dummyParameter;

public:
  ExampleSingleAnalyzer(const Mechanics::Device* device,
                        TDirectory* dir,
                        const char* suffix = "",
                        /* Additional parameters with default values */
                        int dummyParameter = 0,
                        /* Histogram options */
                        unsigned int histBins = 100);

  void processEvent(const Storage::Event* event);
  void postProcessing();
};

}

#endif
