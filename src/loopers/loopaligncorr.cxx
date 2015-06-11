#include <iostream>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <algorithm>

#include <TF1.h>
#include <TH1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TCanvas.h>

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

void prefit(
    TH1& hist,
    double& scale,
    double& mode,
    double& hwhm,
    double& bg) {
  const Int_t nbins = hist.GetNbinsX();
  const Int_t imax = hist.GetMaximumBin();
  scale = hist.GetBinContent(imax);

  Int_t width = 1;
  while (imax+width <= nbins || imax-width >= 1) {
    const Int_t up = std::min(imax+width, nbins);
    const Int_t down = std::max(imax-width, 1);
    if (hist.GetBinContent(up) <= scale/2.) break;
    if (hist.GetBinContent(down) <= scale/2.) break;
    width += 1;
  }

  mode = hist.GetBinCenter(imax);
  hwhm = hist.GetBinWidth(imax)*width;
  const Int_t up = std::min(imax+3*width, nbins);
  const Int_t down = std::max(imax-3*width, 1);
  bg = hist.GetBinContent(up)/2. + hist.GetBinContent(down)/2.;
}

void LoopAlignCorr::finalize() {
  Looper::finalize();

  // For each sensor, index of reference sensor relative to which its residuals
  // were computed
  const std::vector<size_t>& relative = m_correlations.getRelative();
  // The offsets computed for each sensor
  std::vector<double> offX(relative.size(), 0);
  std::vector<double> offY(relative.size(), 0);

  // Gaussian distrubiton fitting function from -5 to +5 sigma
  TF1 gaus("g1", "[0]+[1]*exp(-(x-[2])*(x-[2])/(2*[3]*[3]))", -5, 5);

  size_t iglobal = 0;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    Mechanics::Device& device = *m_devices[idevice];
    for (size_t isens = 0; isens < device.getNumSensors(); isens++) {
      enum Axis { XAXIS, YAXIS };
      for (int axis = XAXIS; axis <= YAXIS; axis++) {
        TH1& hist = (axis==XAXIS) ?
            (TH1&)m_correlations.getResX(idevice, isens) :
            (TH1&)m_correlations.getResY(idevice, isens);

        double scale, mode, hwhm, bg;
        prefit(hist, scale, mode, hwhm, bg);

        gaus.SetParameter(0, bg);
        gaus.SetParameter(1, scale);
        gaus.SetParameter(2, mode);
        gaus.SetParameter(3, hwhm);

        TCanvas* can = 0;
        if (m_draw) {
          can = new TCanvas();
          hist.Draw();
        }

        // N: no plotting
        // Q: quiet
        // S: store results
        // L: likelihood (instead of chi^2)
        TFitResultPtr fit = hist.Fit(
            &gaus, m_draw?"QS":"NQS", "", mode-2*hwhm, mode+2*hwhm);

        assert(iglobal < relative.size() && "More correlations than sensors");

        if (axis==XAXIS)
          offX[iglobal] = fit->Parameter(2);
        else
          offY[iglobal] = fit->Parameter(2);

        if (m_draw) {
          hist.GetXaxis()->SetRangeUser(mode-5*hwhm, mode+5*hwhm);
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

