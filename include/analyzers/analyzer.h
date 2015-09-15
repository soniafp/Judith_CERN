#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include <list>
#include <string>

#include <TDirectory.h>
#include <TH1.h>

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

/**
  * Analyze the events provided at each execute call. Typically a looper will
  * call `execute` at each loop iteration. The analysis can be accumulated into
  * some internal objects such as histograms.
  *
  * Optionall accepts a list of devices which are being analyzed, to complement
  * the data passed by the events. Note that the devices and event objects are
  * const. Analyzers do not modify existing data.
  *
  * Very similar to processors, except they have some interna storage
  * mechanisms and do not modify existing information.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Analyzer {
protected:
  /** Devices being analyzed. One event will be passed for each device at
    * each execute call */
  const std::vector<const Mechanics::Device*> m_devices;
  /** Number of devices being analyzed (also number of events) */
  const size_t m_ndevices;
  /** The events for each device, updated at each execute */
  std::vector<const Storage::Event*> m_events;
  /** Remember if this analyzer has already been finalized. */
  bool m_finalized;

  /** List of all histograms managed by this analyzer. Note that all histograms
    * cast to TH1. */
  std::list<TH1*> m_histograms;
  
  /** Event processing must be specified in derived class */
  virtual void process() = 0;

public:
  /** Constructor for an analyzer which needs device information */
  Analyzer(const std::vector<Mechanics::Device*>& devices);
  /** Constructor for const devices */
  Analyzer(const std::vector<const Mechanics::Device*>& devices);
  /** Convenience constructor for single device */
  Analyzer(const Mechanics::Device& device);
  virtual ~Analyzer();

  /** Set a ROOT output where to store ROOT objects. Call after filling 
    * `m_histograms`, so that it can set those histograms' directories. */
  virtual void setOutput(TDirectory* dir, const std::string& name="");

  /** Multi device execution */
  void execute(const std::vector<Storage::Event*>& events);
  /** Multi device execution with const events */
  void execute(const std::vector<const Storage::Event*>& events);
  /** Single device event execution */
  void execute(const Storage::Event& event);

  /** Post processing */
  virtual void finalize();
};

}

#endif  // ANALYZER_H
