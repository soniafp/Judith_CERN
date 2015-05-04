#include <iostream>
#include <stdexcept>
#include <cmath>
#include <fstream>

#include <TSystem.h>

#include "mechanics/sensor.h"
#include "mechanics/device.h"
#include "mechanics/parsedevice.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_parsing() {
  std::ofstream out("tmp.txt");
  out << 
    "name dut\n"
    "time-unit \\mus\n"
    "space-unit \\mum\n"
    "clock 40\n"
    "window 16\n"
    "\n"
    "sensor:\n"
    "name Diamond\n"
    "chip fei42\n"
    "off-z 0\n"
    "\n"
    "sensor:\n"
    "chip fei4\n"
    "off-x 10000\n"
    "off-y 30000\n"
    "off-z -20000\n"
    "rot-x 3.14159\n"
    "rot-y 3.\n"
    "rot-z .1\n"
    "\n"
    "chip:\n"
    "name fei4\n"
    "rows 336\n"
    "cols 80\n"
    "row-pitch 50\n"
    "col-pitch 250\n"
    "\n"
    "chip:\n"
    "name fei42\n"
    "rows 1000\n"
    "\n" << std::flush;
  out.close();

  Mechanics::Device* device = Mechanics::parseDevice("tmp.txt");

  if (device->m_name != "dut") {
    std::cerr << "Device name incorrect" << std::endl;
    return -1;
  }
  if (device->m_timeUnit != "\\mus") {
    std::cerr << "Device time unit incorrect" << std::endl;
    return -1;
  }
  if (device->m_spaceUnit != "\\mum") {
    std::cerr << "Device space unit incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->m_clockRate, 40)) {
    std::cerr << "Device clock rate incorrect" << std::endl;
    return -1;
  }
  if (device->m_readOutWindow != 16) {
    std::cerr << "Device clock rate incorrect" << std::endl;
    return -1;
  }

  if (device->getSensor(0).m_name != "Sensor0") {
    std::cerr << "Device sensor 0 name incorrect" << std::endl;
    return -1;
  }
  if (device->getSensor(0).m_nrows != 336) {
    std::cerr << "Device sensor 0 nrows incorrect" << std::endl;
    return -1;
  }
  if (device->getSensor(0).m_ncols != 80) {
    std::cerr << "Device sensor 0 ncols incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).m_rowPitch, 50)) {
    std::cerr << "Device sensor 0 rowPitch incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).m_colPitch, 250)) {
    std::cerr << "Device sensor 0 colPitch incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getOffX(), 10000)) {
    std::cerr << "Device sensor 0 OffX incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getOffY(), 30000)) {
    std::cerr << "Device sensor 0 OffY incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getOffZ(), -20000)) {
    std::cerr << "Device sensor 0 OffZ incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getRotX(), 3.14159)) {
    std::cerr << "Device sensor 0 RotX incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getRotY(), 3.)) {
    std::cerr << "Device sensor 0 RotY incorrect" << std::endl;
    return -1;
  }
  if (!approxEqual(device->getSensor(0).getRotZ(), .1)) {
    std::cerr << "Device sensor 0 RotZ incorrect" << std::endl;
    return -1;
  }

  if (device->getSensor(1).m_name != "Diamond") {
    std::cerr << "Device sensor 1 name incorrect" << std::endl;
    return -1;
  }
  if (device->getSensor(1).m_nrows != 1000) {
    std::cerr << "Device sensor 1 nrows incorrect" << std::endl;
    return -1;
  }
  
  delete device;

  gSystem->Exec("rm -f tmp.txt");
  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_parsing()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
