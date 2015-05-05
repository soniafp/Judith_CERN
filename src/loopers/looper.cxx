#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <TStopwatch.h>

#include "storage/storagei.h"
#include "storage/event.h"
#include "loopers/looper.h"

namespace Loopers {

Looper::Looper() :
    m_inputs(),
    m_events(),
    m_maxEvents(0),
    m_minEvents(-1),  // largest unsigned integer
    m_start(0),
    m_nprocess(-1),  // causes iteration over entire range
    m_nstep(1),
    m_printInterval(1E4) {}

void Looper::addInput(Storage::StorageI& input) {
  m_inputs.push_back(&input);
  m_events.push_back(0);  // one for each input
  // Keep track of the available event range
  m_minEvents = std::min(m_minEvents, (ULong64_t)input.getNumEvents());
  m_maxEvents = std::max(m_maxEvents, (ULong64_t)input.getNumEvents());
}

void Looper::printProgress() {
  const ULong64_t nelapsed = m_ievent - m_start;
  const double telapsed = m_timer.RealTime();
  m_timer.Continue();
  const double bandwidth = telapsed*1E6 / (double)nelapsed;
  const double progress = nelapsed / (double)m_nprocess;
  
  std::printf("\r[");  // \r overwrite the last line
  const size_t size = 50;  // number of = symbols in bar
  for (size_t i = 0; i < size; i++)
    // print = for the positions below the current progress
    std::printf((progress*50>i) ? "=" : " ");
  std::printf("] ");
  // Print also the % done, and the bandwidth
  std::printf("%3d%%, %5.1f us", (int)(progress*100), bandwidth);
  // Show on the output (don't buffer)
  std::cout << std::flush;
}

void Looper::loop() {
  // If no number of events is requested, default to all
  if (m_nprocess == (ULong64_t)(-1))
    m_nprocess = m_minEvents - m_start;

  // Check that the given event range works
  if (m_start >= m_minEvents)
    throw std::runtime_error("Looper::loop: start event out of range");
  if (m_start+m_nprocess > m_minEvents)
    throw std::runtime_error("Looper::loop: nprocess exceeds range");
  if (m_nstep < 1)
    throw std::runtime_error("Looper::loop: step size can't be smaller than 1");
  
  for (m_ievent = m_start; m_ievent < m_start+m_nprocess; m_ievent += m_nstep) {
    // If a print interval is given, and this event is on it, print progress
    if (m_printInterval && (m_ievent%m_printInterval == 0)) printProgress();
    // Read this event from each input file
    for (size_t i = 0; i < m_inputs.size(); i++)
      m_events[i] = &m_inputs[i]->readEvent(m_ievent);
    // Execute this looper's event code
    execute();
  }
  // Print the 100% progress and finish that line
  printProgress();
  std::cout << std::endl;
}

}

