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
#include "analyzers/clusterresiduals.h"
#include "loopers/loopaligncorr.h"

namespace Loopers {

LoopAlignCorr::LoopAlignCorr(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs),
    m_devices(devices),
    m_residuals(m_devices) {
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
  m_residuals.setOutput(0);
  addAnalyzer(m_residuals);
}

LoopAlignCorr::LoopAlignCorr(
    Storage::StorageI& input,
    Mechanics::Device& device) :
    Looper(input),
    m_devices(1, &device),
    m_residuals(device) {
  if (device.getNumSensors() != input.getNumPlanes())
    throw std::runtime_error(
        "LoopAlignCorr::loop: device / input plane mismatch");
  m_residuals.setOutput(0);
  addAnalyzer(m_residuals);
}

void LoopAlignCorr::finalize() {
  Looper::finalize();

  // Count the sensor ahead of time to ensure consistency
  size_t nsensors = 0;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++)
    nsensors += m_devices[idevice]->getNumSensors();

  // Offset of each sensor w.r.t. to its reference
  std::vector<double> offX(nsensors, 0);
  std::vector<double> offY(nsensors, 0);
  // Index of the reference sensor
  std::vector<size_t> relativeX(nsensors, 0);
  std::vector<size_t> relativeY(nsensors, 0);

  assert(!m_devices.empty());
  const Mechanics::Device& refDevice = *m_devices[0];

  // Global sensor index in all devices
  size_t iglobal = -1;

  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    Mechanics::Device& device = *m_devices[idevice];

    for (size_t isens = 0; isens < device.getNumSensors(); isens++) {
      iglobal += 1;

      // First reference doesn't align, keep values at 0
      if (idevice == 0 && isens == 0) continue;

      enum Axis { XAXIS, YAXIS };
      for (int axis = XAXIS; axis <= YAXIS; axis++) {

        // Best residual histogram
        TH1* bestHist = 0;
        size_t ibest = 0;
        double bestSigma = 0;
        double bestMean = 0;

        for (size_t iref = 0; iref < refDevice.getNumSensors(); iref++) {
          // Do not allow reference sensors to align to themselves or a sensor
          // further down the device. Note that DUT sensors do not have 
          // residuals w.r.t. to themselves, but only to reference sensors.
          if (idevice == 0 && iref >= isens) continue;

          TH1& hist = (axis==XAXIS) ?
              (TH1&)m_residuals.getResidualX(idevice, isens, iref) :
              (TH1&)m_residuals.getResidualY(idevice, isens, iref);

          double mean = 0;
          double sigma = 1;
          double norm = 1;
          double bg = 0;
          Utils::fitGausBg(hist, mean, sigma, norm, bg, true, false);
          
          if (sigma < bestSigma || iref == 0) {
            bestHist = &hist;
            ibest = iref;
            bestSigma = sigma;
            bestMean = mean;
          }
        }  // reference sensor loop

        assert(bestHist && "Didn't find a residual for alignment");

        if (axis==XAXIS) {
          offX[iglobal] = bestMean;
          relativeX[iglobal] = ibest;
        }
        else {
          offY[iglobal] = bestMean;
          relativeY[iglobal] = ibest;
        }

        if (m_draw) {
          TCanvas* can = m_draw ? new TCanvas() : 0;

          bestHist->Draw();

          // re-do the fit so it can be drawn
          double mean = 0;
          double sigma = 1;
          double norm = 1;
          double bg = 0;
          Utils::fitGausBg(*bestHist, mean, sigma, norm, bg, true, true);

          const double range = 5*sigma;
          bestHist->GetXaxis()->SetRangeUser(mean-range, mean+range);
          bestHist->GetYaxis()->SetRangeUser(0, bestHist->GetMaximum()*1.5);
          can->Modified();
          can->Update();
          can->WaitPrimitive();
        }
      }  // axis loop
    }  // sensor loop
  }  // device loop

  assert(iglobal == nsensors-1 && "Didn't get one offset per sensor");
  
  // Apply each sensor's alignment & its reference sensor alignment. Note that
  // since all DUT sensors are relative to the reference, and the reference
  // sensors are first to process, then processing in order ensures correct
  // propagation of relative offsets.

  // Accumulate the relative offsets
  for (size_t i = 1; i < nsensors; i++) {
    offX[i] += offX[relativeX[i]];
    offY[i] += offY[relativeY[i]];
  }

  iglobal = -1;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    Mechanics::Device& device = *m_devices[idevice];
    for (size_t isens = 0; isens < device.getNumSensors(); isens++) {
      iglobal += 1;
      if (idevice+isens > 0) {
        // Get the sensor to align
        Mechanics::Sensor& sensor = device.getSensor(isens);
        sensor.setOffX(sensor.getOffX() - offX[iglobal]);
        sensor.setOffY(sensor.getOffY() - offY[iglobal]);
      }
    }
  }

  assert(iglobal == nsensors-1 && "Didn't get one offset per sensor");
}

}

