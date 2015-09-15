#include <iostream>
#include <stdexcept>
#include <vector>

#include "processors/processor.h"

namespace Processors {

Processor::Processor(size_t ndevices) :
    // No device information used by this analyzer
    m_devices(),
    m_ndevices(ndevices),
    // Reserve the event pointer memory ahead of time
    m_events(m_ndevices, 0) {
  if (m_ndevices == 0)
    throw std::runtime_error("Processor::Processor: can't expect 0 events");
}

Processor::Processor(const std::vector<Mechanics::Device*>& devices) :
    // Can't directly cast vector to vector of consts, initialize from iterators
    m_devices(devices.begin(), devices.end()),
    m_ndevices(m_devices.size()),
    m_events(m_ndevices, 0) {
  if (m_ndevices == 0)
    throw std::runtime_error("Processor::Processor: empty device vector");
}

Processor::Processor(Mechanics::Device& device) :
    // Single device, so create a 1 item vector filled with its address
    m_devices(1, &device),
    m_ndevices(1),
    m_events(m_ndevices, 0) {}

void Processor::execute(const std::vector<Storage::Event*>& events) {
  // Check that one event is passed for each device or presumed device
  if (events.size() != m_ndevices)
    throw std::runtime_error("Processor::execute: incorrect number of events passed");
  // Copy the event pointers into internal memory
  for (size_t i = 0; i < m_ndevices; i++)
    m_events[i] = events[i];
  // Call the derived class' process code on the events
  process();
}

void Processor::execute(Storage::Event& event) { 
  if (m_ndevices != 1)
    throw std::runtime_error("Processor::execute: incorrect number of events passed");
  m_events[0] = &event;
  process();
}

}

