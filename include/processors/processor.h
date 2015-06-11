#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <vector>

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Processors {

/**
  * Process the events provided at each execute call. Typically a looper will
  * call `execute` at each loop iteration. The processor can then modify the
  * event data, and possibly device information. A post processing method is
  * available to use accumulated data to make further changes to devices.
  *
  * Optionall accepts a list of devices which are being proceseed, to complement
  * the data passed by the events.
  *
  * Very similar to analyzers, but they do modify data, and do not have a
  * default internal storage mechanism.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Processor {
protected:
  /** Optional vector of device information. Note: it is up to the derived 
    * analyzer to check if device information is provided. */
  const std::vector<Mechanics::Device*> m_devices;
  /** Number of devices being analyzed (also number of events) */
  const size_t m_ndevices;
  /** The events for each device, updated at each execute */
  std::vector<Storage::Event*> m_events;
  
  /** Event processing must be specified in derived class */
  virtual void process() = 0;

public:
  /** Constructor for an analyzer which doesn't need devices, only events */
  Processor(size_t ndevices);
  /** Constructor for an analyzer which needs device information */
  Processor(const std::vector<Mechanics::Device*>& devices);
  /** Convenience constructor for single device */
  Processor(Mechanics::Device& device);
  virtual ~Processor() {}

  /** Multi device execution */
  void execute(const std::vector<Storage::Event*>& events);
  /** Single device event execution */
  void execute(Storage::Event& event);

  /** Post processing */
  virtual void finalize() {}
};

}

#endif  // PROCESSOR_H

