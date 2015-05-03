#include <iostream>
#include <stdexcept>
#include <cmath>

#include "mechanics/alignment.h"
#include "mechanics/sensor.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_pixelToSpace() {
  Mechanics::Sensor sensor;
  sensor.m_nrows = 2;
  sensor.m_ncols = 2;
  sensor.m_rowPitch = 1;
  sensor.m_colPitch = 1;

  double x, y, z;

  sensor.pixelToSpace(0, 0, x, y, z);
  if (x != -.5 || y != -.5 || z != 0) {
    std::cerr << "pixelToSpace failed to transform 0,0" << std::endl;
    return -1;
  }

  sensor.pixelToSpace(1, 0, x, y, z);
  if (x != +.5 || y != -.5 || z != 0) {
    std::cerr << "pixelToSpace failed to transform 0,1" << std::endl;
    return -1;
  }

  sensor.pixelToSpace(1, 1, x, y, z);
  if (x != +.5 || y != +.5 || z != 0) {
    std::cerr << "pixelToSpace failed to transform 1,1" << std::endl;
    return -1;
  }

  return 0;
}

int test_spaceToPixel() {
  Mechanics::Sensor sensor;
  sensor.m_nrows = 2;
  sensor.m_ncols = 2;
  sensor.m_rowPitch = 1;
  sensor.m_colPitch = 1;

  double row, col;

  sensor.spaceToPixel(-.5, -.5, 0, col, row);
  if (row != 0 || col != 0) {
    std::cerr << "spaceToPixel failed to transform to 0,0" << std::endl;
    return -1;
  }

  sensor.spaceToPixel(-1, -1, 0, col, row);
  if (row != -0.5 || col != -0.5) {
    std::cerr << "spaceToPixel failed to transform to -.5,-.5" << std::endl;
    return -1;
  }

  sensor.spaceToPixel(-1.5, -1, 0, col, row);
  if (row != -0.5 || col != -1) {
    std::cerr << "spaceToPixel failed to transform to -.5,-1" << std::endl;
    return -1;
  }

  return 0;
}

int test_transformations() {
  // NOTE: the transformations are tested in the alignment tests. This is more
  // of a sanity check, and it isn't exhaustive

  Mechanics::Sensor sensor;
  sensor.m_nrows = 2;
  sensor.m_ncols = 2;
  sensor.m_rowPitch = 1;
  sensor.m_colPitch = 1;

  sensor.setOffX(1);
  sensor.setOffY(-1);
  sensor.setRotZ(M_PI/6);

  double row, col;
  double x, y, z;

  // Local point: 0.5, -0.5. Rotate that by 30 degrees, then offset
  sensor.pixelToSpace(1, 0, x, y, z);
  if (!approxEqual(x, 0.5*std::cos(M_PI/6) - -0.5*std::sin(M_PI/6) + 1) ||
      !approxEqual(y, 0.5*std::sin(M_PI/6) + -0.5*std::cos(M_PI/6) - 1)) {
    std::cerr << "pixelToSpace failed with z rotationad and offset" << std::endl;
    return -1;
  }

  sensor.setRotZ(0);
  sensor.setRotX(M_PI/6);

  // The point 1 unit away in z from the 1,0 pixel before offset
  x = 0.5;
  y = -0.5;
  z = 1;

  // Tilt it along the x-axis as will be applied to the sensor
  Mechanics::Alignment tilt;
  tilt.setRotX(M_PI/6);
  tilt.transform(x, y, z);

  // Apply the sensor offset
  x += 1;
  y -= 1;

  // Get the pixel coordinates of this point (should project back to 1,0)
  sensor.spaceToPixel(x, y, z, col, row);

  if (!approxEqual(col, 1) || !approxEqual(row, 0)) {
    std::cerr << "spaceToPixel failed with x tilt and offset" << std::endl;
    return -1;
  }

  // Closure test with asymmetric transformation
  sensor.setOffX(.1);
  sensor.setOffY(-0.2);
  sensor.setOffZ(3.3);
  sensor.setRotX(0.2);
  sensor.setRotY(-0.1);
  sensor.setRotZ(4.1);

  sensor.pixelToSpace(1, 0, x, y, z);
  sensor.spaceToPixel(x, y, z, col, row);

  if (!approxEqual(col, 1) || !approxEqual(row, 0)) {
    std::cerr << "Transformation closure failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_pixelToSpace()) != 0) return retval;
    if ((retval = test_spaceToPixel()) != 0) return retval;
    if ((retval = test_transformations()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
