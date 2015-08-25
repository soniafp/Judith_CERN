#ifndef LOOPER_H
#define LOOPER_H

#include <vector>

#include <Rtypes.h>

namespace Storage { class StorageIO; }
namespace Analyzers { class SingleAnalyzer; }
namespace Analyzers { class DualAnalyzer; }

namespace Loopers {

class Looper
{
protected:
  Storage::StorageIO* _refStorage;
  Storage::StorageIO* _dutStorage;
  ULong64_t _startEvent;
  ULong64_t _numEvents;
  Long64_t _eventSkip;
  ULong64_t _totalEvents;
  ULong64_t _endEvent;

  std::vector<Analyzers::SingleAnalyzer*> _singleAnalyzers;
  unsigned int _numSingleAnalyzers;
  std::vector<Analyzers::DualAnalyzer*> _dualAnalyzers;
  unsigned int _numDualAnalyzers;

  Looper(Storage::StorageIO* refStorage,
         Storage::StorageIO* dutStorage = 0,
         ULong64_t startEvent = 0,
         ULong64_t numEvents = 0,
         Long64_t eventSkip = 1);
  virtual ~Looper();

  void progressBar(ULong64_t nevent);

public:
  static bool noBar;

  void addAnalyzer(Analyzers::SingleAnalyzer* analyzer);
  void addAnalyzer(Analyzers::DualAnalyzer* analyzer);

  ULong64_t getStartEvent() const { return _startEvent; }
  ULong64_t getEndEvent() const { return _endEvent; }
};

}

#endif // LOOPER_H
