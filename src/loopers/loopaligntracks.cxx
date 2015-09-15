#include <iostream>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <list>

#include <Rtypes.h>
#include <Math/Minimizer.h>
#include <Math/Factory.h>

#include "utils.h"
#include "mechanics/device.h"
#include "analyzers/analyzer.h"
#include "analyzers/trackchi2.h"
#include "loopers/loopaligntracks.h"

namespace Loopers {

LoopAlignTracks::LoopAlignTracks(
    const std::vector<Storage::StorageI*>& inputs,
    const std::vector<Mechanics::Device*>& devices) :
    Looper(inputs, devices),
    m_trackChi2(devices),
    // Tracking for only the reference device. Note that Looper::Looper ensures
    // at least one device in the vector
    m_tracking(devices[0]->getNumSensors()) {
  if (m_devices.size() > 2)
    throw std::runtime_error(
        "LoopAlignTracks::LoopAlignTracks: supports at most two devices");
  // Let the base class deal with the analyzer
  addAnalyzer(m_trackChi2);
}

LoopAlignTracks::LoopAlignTracks(
    Storage::StorageI& input,
    Mechanics::Device& device) :
    Looper(input, device),
    m_trackChi2(device),
    m_tracking(device.getNumSensors()) {
  addAnalyzer(m_trackChi2);
}

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

double LoopAlignTracks::Chi2Minimizer::DoEval(const double* pars) const {
  double sum = 0;

  m_device->setAlignment(pars);

  for (std::vector<Analyzers::TrackChi2::Track>::const_iterator
      it = m_tracks.begin(); it != m_tracks.end(); ++it) {
    const Analyzers::TrackChi2::Track& track = *it;

    const size_t n = track.constituents.size();
    std::vector<double> x(n);
    std::vector<double> xe(n);
    std::vector<double> y(n);
    std::vector<double> ye(n);
    std::vector<double> z(n);

    for (size_t i = 0; i < n; i++) {
      const Analyzers::TrackChi2::Cluster& cluster = 
          m_clusters[track.constituents[i]];
      cluster.sensor->pixelToSpace(
          cluster.pixX, cluster.pixY, x[i], y[i], z[i]);
      // TODO: single set rotation scale
      double ex, ey, ez;
      cluster.sensor->pixelToSpace(
          cluster.pixErrX, cluster.pixErrY,
          ex, ey, ez);
      xe[i] = std::fabs(ex-x[i]);
      ye[i] = std::fabs(ey-y[i]);
    }

    double chi2;

    double p0x, p1x, p0ex, p1ex, covx;
    Utils::linearFit(
        n, &z[0], &x[0], &xe[0], p0x, p1x, p0ex, p1ex, covx, chi2);

    double p0y, p1y, p0ey, p1ey, covy;
    Utils::linearFit(
        n, &z[0], &y[0], &ye[0], p0y, p1y, p0ey, p1ey, covy, chi2);

    // TODO: extrapolate track and uncertainty to each constituent or match
  }

  return sum;
}

ROOT::Math::IBaseFunctionMultiDim* LoopAlignTracks::Chi2Minimizer::Clone() const {
  return new LoopAlignTracks::Chi2Minimizer(*this);
}

void LoopAlignTracks::finalize() {
  Looper::finalize();

  // Copy the alignment objects (clusters and tracks) from the anlyzer into
  // continous memory. Hopefully this will help with caching.
  Chi2Minimizer minEval(
      *m_devices.back(),
      m_trackChi2.getClusters(),
      m_trackChi2.getTracks());

  // Build the default minimizer (probably Minuit with Migrad)
  ROOT::Math::Minimizer* minimizer = ROOT::Math::Factory::CreateMinimizer();

  minimizer->SetFunction(minEval);

  // TODO: run evaluation, and set parameters in device

  delete minimizer;
}

}

