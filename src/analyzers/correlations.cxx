#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>

#include <TDirectory.h>
#include <TH1D.h>
#include <TH2D.h>

#include "storage/event.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "mechanics/device.h"
#include "mechanics/sensor.h"
#include "analyzers/correlations.h"

namespace Analyzers {

void Correlations::initialize() {
  // Sort all device planes by z-distance
  assert(!m_devices.empty());

  size_t nsensors = 0;
  for (size_t i = 0; i < m_devices.size(); i++)
    nsensors += m_devices[i]->getNumSensors();

  if (nsensors == 0)
    throw std::runtime_error("LoopAlignCorr::loop: no planes to align");

  // Each plane gets the index of another plane to which it aligns
  m_irelative.assign(nsensors, 0);
  // Also keep a list of all sensors in their global order
  std::vector<const Mechanics::Sensor*> sensors(nsensors, 0);

  // Get the reference device which is used throughout
  assert(!m_devices.empty());
  const Mechanics::Device& ref = *m_devices[0];

  // Global index keeps incrementing within nested device / sensor loops
  size_t iglobal = 0;

  // The reference device is first, its sensor indices are the same as global
  for (iglobal = 0; iglobal < ref.getNumSensors(); iglobal++) {
    // First reference aligns to itself, the rest align to the prior plane
    m_irelative[iglobal] = (iglobal==0) ? 0 : iglobal-1;
    sensors[iglobal] = &ref[iglobal];
  }

  // iglobal has counted all reference planes, can now start on DUTs

  // For each DUT plane, find the nearest preceeding reference plane
  for (size_t idevice = 1; idevice < m_devices.size(); idevice++) {
    const Mechanics::Device& dut = *m_devices[idevice];
    // Loop over DUT device planes
    for (size_t idut = 0; idut < dut.getNumSensors(); idut++) {
      // Loop over reference device planes to find the nearest
      for (size_t iref = 0; iref < ref.getNumSensors(); iref++) {
        // Stop once the DUT plane has been surpassed in z (unless its the
        // first reference plane, in which case use it since the DUT comes
        // before the reference)
        if (ref[iref].getOffZ() > dut[idut].getOffZ() && iref > 0) break;
        // Keep moving the align-to index to the nearest ref sensor
        m_irelative[iglobal] = iref;
        sensors[iglobal] = &dut[iglobal];
      }
      // Move onto the next global DUT sensor for which to find a reference
      iglobal += 1;
    }
  }

  assert(iglobal == nsensors && "Should have iterated all sensors once");

  // Generate the histograms for each sensor
  for (iglobal = 0; iglobal < nsensors; iglobal++) {
    const Mechanics::Sensor& sensor = *sensors[iglobal];
    const Mechanics::Sensor& relative = *sensors[m_irelative[iglobal]];

    // Sensors need not have devices, but they are retrieved from a device
    // in this class, so this is just a sanity check
    assert(sensor.getDevice());

    const Mechanics::Device& device = *sensor.getDevice();
    const std::string& unit = device.m_spaceUnit;
    const double sensorSizeX = sensor.m_colPitch*sensor.m_ncols;
    const double relativeSizeX = sensor.m_colPitch*sensor.m_ncols;
    const double sensorSizeY = sensor.m_rowPitch*sensor.m_nrows;
    const double relativeSizeY = sensor.m_rowPitch*sensor.m_nrows;

    m_hCorrelationsX.push_back(new TH2D(
        ("CorrX_" + device.m_name + "_" + sensor.m_name).c_str(),
        ("CorrX_" + device.m_name + "_" + sensor.m_name).c_str(),
        sensor.m_ncols, 0, sensor.m_ncols,
        relative.m_ncols, 0, relative.m_ncols));
    m_hCorrelationsX.back()->GetXaxis()->SetTitle(
        (sensor.m_name+" col").c_str());
    m_hCorrelationsX.back()->GetYaxis()->SetTitle(
        (relative.m_name+" col").c_str());
    m_hCorrelationsX.back()->GetZaxis()->SetTitle("Triggers");
    // No stats pannel for 2D histogram
    m_hCorrelationsX.back()->SetStats(false);
    // Base class keeps track of all histograms
    m_histograms.push_back(m_hCorrelationsX.back());

    m_hCorrelationsY.push_back(new TH2D(
        ("CorrY_" + device.m_name + "_" + sensor.m_name).c_str(),
        ("CorrY_" + device.m_name + "_" + sensor.m_name).c_str(),
        sensor.m_nrows, 0, sensor.m_nrows,
        relative.m_nrows, 0, relative.m_nrows));
    m_hCorrelationsY.back()->GetXaxis()->SetTitle(
        (sensor.m_name+" row").c_str());
    m_hCorrelationsY.back()->GetYaxis()->SetTitle(
        (relative.m_name+" row").c_str());
    m_hCorrelationsY.back()->GetZaxis()->SetTitle("Triggers");
    m_hCorrelationsY.back()->SetStats(false);
    m_histograms.push_back(m_hCorrelationsY.back());

    m_hResidualsX.push_back(new TH1D(
        ("ResX_" + device.m_name + "_" + sensor.m_name).c_str(),
        ("ResX_" + device.m_name + "_" + sensor.m_name).c_str(),
        sensor.m_ncols+relative.m_ncols,
        -(sensorSizeX+relativeSizeX), (sensorSizeX+relativeSizeY)));
    m_hResidualsX.back()->GetXaxis()->SetTitle(
        ("#Delta x ["+unit+"]").c_str());
    m_hResidualsX.back()->GetZaxis()->SetTitle("Triggers");
    m_histograms.push_back(m_hResidualsX.back());

    m_hResidualsY.push_back(new TH1D(
        ("ResY_" + device.m_name + "_" + sensor.m_name).c_str(),
        ("ResY_" + device.m_name + "_" + sensor.m_name).c_str(),
        sensor.m_nrows+relative.m_nrows,
        -(sensorSizeY+relativeSizeY), (sensorSizeY+relativeSizeY)));
    m_hResidualsY.back()->GetXaxis()->SetTitle(
        ("#Delta y ["+unit+"]").c_str());
    m_hResidualsY.back()->GetZaxis()->SetTitle("Triggers");
    m_histograms.push_back(m_hResidualsY.back());
  }
}

void Correlations::process() {
  // Keep track of the reference event (with all reference planes)
  assert(m_ndevices > 0 && "ensures > 0 events");
  const Storage::Event& refEvent = *m_events[0];

  size_t iglobal = 0;  // global index from all devices
  for (size_t idevice = 0; idevice < m_devices.size(); idevice++) {
    const Storage::Event& event = *m_events[idevice];
    for (size_t iplane = 0; iplane < event.getNumPlanes(); iplane++) {
      // Get the plane to align
      const Storage::Plane& plane = event.getPlane(iplane);
      // Get the plane from the reference device to which to align
      assert(m_irelative[iglobal] < refEvent.getNumPlanes());
      const Storage::Plane& refPlane = refEvent.getPlane(m_irelative[iglobal]);

      // Fill the histograms
      for (size_t icluster = 0; icluster < plane.getNumClusters(); icluster++) {
        const Storage::Cluster& cluster = plane.getCluster(icluster);
        for (size_t iref = 0; iref < refPlane.getNumClusters(); iref++) {
          const Storage::Cluster& refCluster = refPlane.getCluster(iref);
          m_hCorrelationsX[iglobal]->Fill(cluster.getPixX(), refCluster.getPixX());
          m_hCorrelationsY[iglobal]->Fill(cluster.getPixY(), refCluster.getPixY());
          m_hResidualsX[iglobal]->Fill(cluster.getPosX() - refCluster.getPosX());
          m_hResidualsY[iglobal]->Fill(cluster.getPosY() - refCluster.getPosY());
        }  // ref clusters
      }  // plane clusters

      iglobal += 1;  // next global plane
    }  // planes
  }  // devices
}

size_t Correlations::toGlobal(size_t idevice, size_t isensor) const {
  // Simple counting of sensor up to the correct device. Infrequently called,
  // so no need to cache or map indices.
  size_t iglobal = 0;
  for (size_t id = 0; id < idevice; id++)
    iglobal += m_devices[id]->getNumSensors();
  iglobal += isensor;
  return iglobal;
}

const std::vector<size_t>& Correlations::getRelative() const {
  return m_irelative;
}

TH2D& Correlations::getCorrX(size_t idevice, size_t isensor) const {
  return *m_hCorrelationsX[toGlobal(idevice, isensor)];
}

TH2D& Correlations::getCorrY(size_t idevice, size_t isensor) const {
  return *m_hCorrelationsY[toGlobal(idevice, isensor)];
}

TH1D& Correlations::getResX(size_t idevice, size_t isensor) const {
  return *m_hResidualsX[toGlobal(idevice, isensor)];
}

TH1D& Correlations::getResY(size_t idevice, size_t isensor) const {
  return *m_hResidualsY[toGlobal(idevice, isensor)];
}

}

