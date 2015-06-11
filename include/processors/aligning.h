#ifndef PROC_ALIGN_H
#define PROC_ALIGN_H

#include "processors/processor.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Processors {

/**
  * Applies the alignment from a device to the objects in the events.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Aligning : public Processor {
protected:
  /** Processing is done device-by-device, so make single device method */
  void processEvent(
      Storage::Event& event,
      const Mechanics::Device& device);

  /** Base virtual method called at each loop iteration */
  virtual void process();

public:
  /** Enable multi-device initialization */
  Aligning(const std::vector<Mechanics::Device*>& devices) :
      Processor(devices) {}
  /** Enable single-device initialization */
  Aligning(Mechanics::Device& device) :
      Processor(device) {}
  virtual ~Aligning() {}
};

}

#endif  // PROC_ALIGN_H
