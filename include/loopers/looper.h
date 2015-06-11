#ifndef LOOPER_H
#define LOOPER_H

#include <vector>

#include <Rtypes.h>
#include <TStopwatch.h>

namespace Storage { class StorageI; }
namespace Storage { class Event; }
namespace Processors { class Processor; }
namespace Analyzers { class Analyzer; }

namespace Loopers {

/**
  * Base class for objects which loop over the events in a set of input and
  * perform some action on each event. Contains code to setup and execute the
  * loop over an event range, and display pgoress.
  *
  * Can accomodate procssors and analyzers by default. Derived classes can
  * introduce more specific functionality.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Looper {
private:
  // No copy or assignment, otherwise need to hande analyzers and processors
  Looper(const Looper&);
  Looper& operator=(const Looper&);

protected:
  /** List of inputs from which to read events */
  const std::vector<Storage::StorageI*> m_inputs;
  /** List of events (in the same order as the inputs) read an iteration */
  std::vector<Storage::Event*> m_events;
  /** The largets event index from the inputs */
  ULong64_t m_maxEvents;
  /** The smallest last event index from the inputs */
  ULong64_t m_minEvents;

  /** Track the current event globally for the progress bar */
  ULong64_t m_ievent;
  /** Timer keeps track of time between progress bar calls */
  TStopwatch m_timer;

  /** List of processors to execute at each loop. Not owned by this. */
  std::vector<Processors::Processor*> m_processors;
  /** List of analyzers to execute at each loop. Not owned by this. */
  std::vector<Analyzers::Analyzer*> m_analyzers;

  /** Print a progress bar and bandwidth */
  void printProgress();

public:
  /** First event index to process */
  ULong64_t m_start;
  /** Process up to this many events after the first */
  ULong64_t m_nprocess;
  /** Skip this many events at each iteration */
  ULong64_t m_nstep;
  /** Print the progress bar at this interval of events (0 is off */
  unsigned m_printInterval;
  /** Draw outputs or not (not always applicable) */
  bool m_draw;

  Looper(const std::vector<Storage::StorageI*>& inputs);
  /** Convenience constructor for single input loopers */
  Looper(Storage::StorageI& input);
  virtual ~Looper() {}
  
  /** Loop over the largest common set of events in the inputs */
  virtual void loop();
  /** Execute processors and analyzers */
  virtual void execute();
  /** Post processing */
  virtual void finalize();

  /** Add a processor to execute at each loop iteration */
  void addProcessor(Processors::Processor& processor);
  /** Add an analyzer to execute at each loop iteration */
  void addAnalyzer(Analyzers::Analyzer& analyzer);
};

}

#endif  // LOOPER_H
