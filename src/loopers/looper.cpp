#include "looper.h"

#include <cassert>
#include <vector>
#include <iostream>
#include <iomanip>

#include <Rtypes.h>

#include "../storage/storageio.h"
#include "../analyzers/singleanalyzer.h"
#include "../analyzers/dualanalyzer.h"

using std::cout;
using std::endl;
using std::flush;

namespace Loopers {

bool Looper::noBar = false;

void Looper::progressBar(ULong64_t nevent)
{
  if (noBar) return;

  assert(nevent >= _startEvent && nevent <= _endEvent &&
         "Looper: progress recieved event outside range");

  ULong64_t progress = nevent - _startEvent + 1;
  if (progress % 500 && nevent != _endEvent) return;

  progress = (progress * 100) / _numEvents; // Now as an integer %

  cout << "\rProgress: [";
  for (unsigned int i = 1; i <= 50; i++)
  {

    if (progress >= i * 2) cout << "=";
    else cout << " ";
  }
  cout << "] " << std::setw(4) << progress << "%" << flush;
  if (nevent == _endEvent) cout << endl;
}

void Looper::addAnalyzer(Analyzers::SingleAnalyzer* analyzer)
{
  assert(analyzer && "Looper: tried to add a null analyzer");
  _singleAnalyzers.push_back(analyzer);
  _numSingleAnalyzers++;
}

void Looper::addAnalyzer(Analyzers::DualAnalyzer* analyzer)
{
  assert(analyzer && "Looper: tried to add a null analyzer");
  _dualAnalyzers.push_back(analyzer);
  _numDualAnalyzers++;
}

Looper::Looper(Storage::StorageIO* refStorage,
               Storage::StorageIO* dutStorage,
               ULong64_t startEvent,
               ULong64_t numEvents,
               Long64_t eventSkip) :
  _refStorage(refStorage),
  _dutStorage(dutStorage),
  _startEvent(startEvent),
  _numEvents(numEvents),
  _eventSkip(eventSkip),
  _totalEvents(0),
  _endEvent(0),
  _numSingleAnalyzers(0),
  _numDualAnalyzers(0)
{
  assert(refStorage && "Looper: null ref. storage passed");

  _totalEvents = _refStorage->getNumEvents();
  if (_dutStorage && (Long64_t)_totalEvents > _dutStorage->getNumEvents())
    _totalEvents = _dutStorage->getNumEvents();

  //if (_eventSkip < 1)
  // throw "Looper: event skip can't be smaller than 1";
  if (int(_eventSkip) - int(_startEvent) < 1)
    throw "Looper: event skip can't be smaller than startEvent";  
  if (_startEvent >= _totalEvents)
    throw "Looper: start event exceeds storage file";

  if (_numEvents == 0) _numEvents = _totalEvents - _startEvent;

  _endEvent = _numEvents + _startEvent - 1;
  if (_endEvent >= _totalEvents)
    throw "Looper: end event exceeds storage file";
}

Looper::~Looper()
{
  for (unsigned int i = 0; i < _singleAnalyzers.size(); i++)
    delete _singleAnalyzers.at(i);
  for (unsigned int i = 0; i < _dualAnalyzers.size(); i++)
    delete _dualAnalyzers.at(i);
}

}
