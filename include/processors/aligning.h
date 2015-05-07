#ifndef ALIGN_H
#define ALIGN_H

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Processors {

/**
  * Applies the alignment from a device to the objects in the events.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Aligning {
protected:
  const Mechanics::Device& m_device;

public:
  /** Aligning always applies alignment from a device to its events, so require
    * the device at construction */
  Aligning(const Mechanics::Device& device);
  virtual ~Aligning() {}

  /** Fill event position values with aligned ones */
  virtual void process(Storage::Event& event);
};

}

#endif  // ALIGN_H
