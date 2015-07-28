#ifndef SYNCFLUCTUATION_H
#define SYNCFLUCTUATION_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>
#include <Rtypes.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class SyncFluctuation : public DualAnalyzer
{
private:
   TH1D* _synchronized;
   TH1D* _unsynchronized;

   ULong64_t _lastRef; // Last ref clock
   ULong64_t _lastDut;
   ULong64_t _lastChangeRef;
   ULong64_t _lastChangeDut;
   ULong64_t _counter;

public:
  SyncFluctuation(const Mechanics::Device* refDevice,
                  const Mechanics::Device* dutDevice,
                  TDirectory* dir = 0,
                  const char* suffix = "",
                  double syncRange = 1E-3,
                  double unsyncRange = 10);

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();

  TH1D* getSynchronized();
  TH1D* getUnsynchronized();
};

}

#endif
