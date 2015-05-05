#include <iostream>
#include <stdexcept>

#include "storage/event.h"
#include "storage/storageo.h"
#include "processors/clustering.h"
#include "loopers/loopprocess.h"

namespace Loopers {

LoopProcess::LoopProcess(Storage::StorageO& output) :
    Looper(),
    m_output(output),
    // No clustering algorithm by default, must be set explicitely
    m_clustering(0) {}

void LoopProcess::loop() {
  // Looper is meant to process only 1 input, and generate 1 output
  if (m_inputs.size() != 1)
    throw std::runtime_error("LoopProcess::loop: can handle only 1 input");
  // Continue with the base class loop
  Looper::loop();
}

void LoopProcess::execute() {
  // Note: guaranteed to have 1 event from `loop` verification
  Storage::Event& event = *m_events[0];
  // If a clustering algorithm was given, apply it
  if (m_clustering) m_clustering->process(event);
  // Store the processed event in the output
  m_output.writeEvent(event);
}

}

