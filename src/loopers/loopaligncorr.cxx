#include <iostream>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <algorithm>

#include <TH1.h>
#include <TCanvas.h>

#include "utils.h"
#include "storage/storagei.h"
#include "mechanics/device.h"
#include "analyzers/correlations.h"
#include "loopers/loopaligncorr.h"

namespace Loopers {

LoopAlignCorr::LoopAlignCorr(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs),
    m_devices(devices),
    m_correlations(m_devices) {
  // Need at least one device
  if (m_devices.empty())
    throw std::runtime_error(
        "LoopAlignCorr::LoopAlignCorr: no devices");
  // Check that there is 1 input per device
  if (m_devices.size() != m_inputs.size())
    throw std::runtime_error(
        "LoopAlignCorr::LoopAlignCorr: device / inputs mismatch");
  // Check that the device sensors match the input planes
  for (size_t i = 0; i < m_devices.size(); i++)
    if (m_devices[i]->getNumSensors() != m_inputs[i]->getNumPlanes())
      throw std::runtime_error(
          "LoopAlignCorr::LoopAlignCorr: device / input plane mismatch");
  // The correlations shouldn't be stored, they are used internally only
  m_correlations.setOutput(0);
  addAnalyzer(m_correlations);
}

LoopAlignCorr::LoopAlignCorr(
    Storage::StorageI& input,
    Mechanics::Device& device) :
    Looper(input),
    m_devices(1, &device),
    m_correlations(device) {
  if (device.getNumSensors() != input.getNumPlanes())
    throw std::runtime_error(
        "LoopAlignCorr::loop: device / input plane mismatch");
  m_correlations.setOutput(0);
  addAnalyzer(m_correlations);
}

void LoopAlignCorr::finalize() {
  Looper::finalize();

  // For each sensor, index of reference sensor relative to which its residuals
  // were computed
  const std::vector<size_t>& relative = m_correlations.getRelative();
  // The offsets computed for each sensor
  std::vector<double> offX(relative.size(), 0);
  std::vector<double> offY(relative.size(), 0);

  size_t iglobal = 0;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    Mechanics::Device& device = *m_devices[idevice];
    for (size_t isens = 0; isens < device.getNumSensors(); isens++) {
      enum Axis { XAXIS, YAXIS };
      for (int axis = XAXIS; axis <= YAXIS; axis++) {
        TH1& hist = (axis==XAXIS) ?
            (TH1&)m_correlations.getResX(idevice, isens) :
            (TH1&)m_correlations.getResY(idevice, isens);

        TCanvas* can = m_draw ? new TCanvas() : 0;

        if (m_draw) hist.Draw();

        double mean = 0;
        double sigma = 1;
        double norm = 1;
        double bg = 0;
        Utils::fitGausBg(hist, mean, sigma, norm, bg, true, m_draw);

        assert(iglobal < relative.size() && "More correlations than sensors");

        if (axis==XAXIS)
          offX[iglobal] = mean;
        else
          offY[iglobal] = mean;

        if (m_draw) {
          const double range = 5*sigma;
          hist.GetXaxis()->SetRangeUser(mean-range, mean+range);
          hist.GetYaxis()->SetRangeUser(0, hist.GetMaximum()*1.5);
          can->Modified();
          can->Update();
          can->WaitPrimitive();
        }
      }  // axis loop

      iglobal += 1;
    }  // sensor loop
  }  // device loop

  assert(iglobal == relative.size() && "Correlations and sensors dont' match");

  // Apply each sensor's alignment & its reference sensor alignment. Note that
  // since all DUT sensors are relative to the reference, and the reference
  // sensors are first to process, then processing in order ensures correct
  // propagation of relative offsets.

  // Accumulate the relative offsets
  for (size_t i = 1; i < relative.size(); i++) {
    offX[i] += offX[relative[i]];
    offY[i] += offY[relative[i]];
  }

  iglobal = 0;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    Mechanics::Device& device = *m_devices[idevice];
    for (size_t isens = 0; isens < device.getNumSensors(); isens++) {
      if (idevice+isens > 0) {
        // Get the sensor to align
        Mechanics::Sensor& sensor = device.getSensor(isens);
        sensor.setOffX(sensor.getOffX() - offX[iglobal]);
        sensor.setOffY(sensor.getOffY() - offY[iglobal]);
      }
      iglobal += 1;
    }
  }
}

}

