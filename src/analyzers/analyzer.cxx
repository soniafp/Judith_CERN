#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

#include <TDirectory.h>
#include <TH1.h>

#include "analyzers/analyzer.h"

namespace Analyzers {

// Many constructors follow to allow for different initialization of devices,
// but they all call the initialize method.

Analyzer::Analyzer(size_t ndevices) :
    // No device information used by this analyzer
    m_devices(),
    m_ndevices(ndevices),
    // Reserve the event pointer memory ahead of time
    m_events(m_ndevices, 0) {
  if (m_ndevices == 0)
    throw std::runtime_error("Analyzer::Analyzer: can't expect 0 events");
}

Analyzer::Analyzer(const std::vector<Mechanics::Device*>& devices) :
    // Can't directly cast vector to vector of consts, initialize from iterators
    m_devices(devices.begin(), devices.end()),
    m_ndevices(m_devices.size()),
    m_events(m_ndevices, 0) {
  if (m_ndevices == 0)
    throw std::runtime_error("Analyzer::Analyzer: empty device vector");
}

Analyzer::Analyzer(const std::vector<const Mechanics::Device*>& devices) :
    // Const devices were passed, copy vector
    m_devices(devices),
    m_ndevices(m_devices.size()),
    m_events(m_ndevices, 0) {
  if (m_ndevices == 0)
    throw std::runtime_error("Analyzer::Analyzer: empty device vector");
}

Analyzer::Analyzer(const Mechanics::Device& device) :
    // Single device, so create a 1 item vector filled with its address
    m_devices(1, &device),
    m_ndevices(1),
    m_events(m_ndevices, 0) {}

Analyzer::~Analyzer() {
  for (std::list<TH1*>::iterator it = m_histograms.begin();
      it != m_histograms.end(); ++it)
    // Delete all non-null histograms with null directories (they are not
    // managed by ROOT)
    if (*it && !(*it)->GetDirectory()) delete *it;
}

void Analyzer::setOutput(TDirectory* dir, const std::string& name) {
  TDirectory* out = dir;  // output directory for histograms

  // If a directory and a name are given, make a new directory with that name
  // and set it as the output
  if (dir && !name.empty()) {
    // Check first if it already exists
    if (dir->Get(name.c_str()))
      throw std::runtime_error(
          "Analyzer::setOutput: output name already exists in directory");
    // Make the directory and set it as the output
    out = dir->mkdir(name.c_str());
  }

  // Move all owned histogram to this output
  for (std::list<TH1*>::iterator it = m_histograms.begin();
      it != m_histograms.end(); ++it)
    (*it)->SetDirectory(out);
}

void Analyzer::execute(const std::vector<Storage::Event*>& events) {
  // Check that one event is passed for each device or presumed device
  if (events.size() != m_ndevices)
    throw std::runtime_error("Analyzer::execute: incorrect number of events passed");
  // Copy the event pointers into internal memory
  for (size_t i = 0; i < m_ndevices; i++)
    m_events[i] = events[i];
  // Call the derived class' process code on the events
  process();
}

void Analyzer::execute(const std::vector<const Storage::Event*>& events) {
  if (events.size() != m_ndevices)
    throw std::runtime_error("Analyzer::execute: incorrect number of events passed");
  for (size_t i = 0; i < m_ndevices; i++)
    m_events[i] = events[i];
  process();
}

void Analyzer::execute(const Storage::Event& event) { 
  if (m_ndevices != 1)
    throw std::runtime_error("Analyzer::execute: incorrect number of events passed");
  m_events[0] = &event;
  process();
}

}

