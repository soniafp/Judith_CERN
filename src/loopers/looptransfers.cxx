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
#include "processors/tracking.h"
#include "analyzers/clusterresiduals.h"
#include "loopers/looptransfers.h"

namespace Loopers {

LoopTransfers::LoopTransfers(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs, devices),
    m_residuals(devices) {
  m_residuals.setOutput(0);
  addAnalyzer(m_residuals);
}

LoopTransfers::LoopTransfers(
    Storage::StorageI& input,
    Mechanics::Device& device) :
    Looper(input, device),
    m_residuals(device) {
  m_residuals.setOutput(0);
  addAnalyzer(m_residuals);
}

void LoopTransfers::finalize() {
  Looper::finalize();

  assert(!m_devices.empty());
  const Mechanics::Device& refDevice = *m_devices[0];

  // Count the sensor ahead of time to ensure consistency
  size_t nscales = 0;
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++)
    nscales += m_devices[idevice]->getNumSensors();
  // Each sensor gets a transfer scale to each reference sensor
  nscales *= refDevice.getNumSensors();

  m_scalesX.assign(nscales, 0);
  m_scalesY.assign(nscales, 0);

  size_t iglobal = -1;

  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    for (size_t isens = 0; isens < m_devices[idevice]->getNumSensors(); isens++) {
      for (size_t iref = 0; iref < refDevice.getNumSensors(); iref++) {
        iglobal += 1;
        enum Axis { XAXIS, YAXIS };
        for (int axis = XAXIS; axis <= YAXIS; axis++) {
          TH1& hist = (axis==XAXIS) ?
              (TH1&)m_residuals.getResidualX(idevice, isens, iref) :
              (TH1&)m_residuals.getResidualY(idevice, isens, iref);
          double mean = 0;
          double sigma = 1;
          double norm = 1;
          double bg = 0;
          //TCanvas* can = new TCanvas();
          //hist.Draw();
          //Utils::fitGausBg(hist, mean, sigma, norm, bg, true, true);
          //can->WaitPrimitive();
          Utils::fitGausBg(hist, mean, sigma, norm, bg, true, false);
          if (axis==XAXIS) m_scalesX[iglobal] = sigma;
          else m_scalesY[iglobal] = sigma;
        }
      }
    }
  }
}

void LoopTransfers::apply(Processors::Tracking& tracking) {
  assert(!m_devices.empty());
  const Mechanics::Device& refDevice = *m_devices[0];

  size_t iglobal = -1;

  for (size_t isens = 0; isens < refDevice.getNumSensors(); isens++) {
    for (size_t iref = 0; iref < refDevice.getNumSensors(); iref++) {
      iglobal += 1;
      tracking.setTransitionX(iref, isens, m_scalesX[iglobal]);
      tracking.setTransitionY(iref, isens, m_scalesY[iglobal]);
    }
  }
}

}

