#include <iostream>
#include <stdexcept>
#include <cassert>
#include <vector>

#include "utils.h"
#include "mechanics/device.h"
#include "analyzers/analyzer.h"
#include "loopers/loopaligntracks.h"

namespace Loopers {

LoopAlignTracks::LoopAlignTracks(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs, devices),
    // Tracking for only the reference device. Note that Looper::Looper ensures
    // at least one device in the vector
    m_tracking(devices[0]->getNumSensors()) {}

LoopAlignTracks::LoopAlignTracks(
    Storage::StorageI& input,
    Mechanics::Device& device) :
    Looper(input, device),
    m_tracking(device.getNumSensors()) {}

void LoopAlignTracks::execute() {
  for (std::vector<Processors::Processor*>::iterator it = m_processors.begin();
      it != m_processors.end(); ++it)
    (*it)->execute(m_events);

  // Tracking needs to be called after any other processors, and only on the
  // reference device
  m_tracking.execute(*m_events[0]);

  for (std::vector<Analyzers::Analyzer*>::iterator it = m_analyzers.begin();
      it != m_analyzers.end(); ++it)
    (*it)->execute(m_events);
}

void LoopAlignTracks::finalize() {
  Looper::finalize();
}

}

