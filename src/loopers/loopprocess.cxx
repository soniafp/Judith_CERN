#include <iostream>
#include <stdexcept>
#include <cassert>

#include "storage/event.h"
#include "storage/storageo.h"
#include "processors/clustering.h"
#include "processors/aligning.h"
#include "loopers/loopprocess.h"

namespace Loopers {

LoopProcess::LoopProcess(
    Storage::StorageI& input,
    Storage::StorageO& output) :
    Looper(input),
    m_output(output) {}

void LoopProcess::execute() {
  Looper::execute();  // run the processors
  // Store the processed event in the output
  assert(m_events.size() == 1 && "Can construct with 1 input only");
  m_output.writeEvent(*m_events[0]);
}

}

