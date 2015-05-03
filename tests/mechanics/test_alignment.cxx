#include <iostream>
#include <stdexcept>
#include <cmath>

#include "mechanics/alignment.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_setGet() {
  Mechanics::Alignment align;

  if (align.getOffX() != 0 ||
      align.getOffY() != 0 ||
      align.getOffZ() != 0 ||
      align.getRotX() != 0 ||
      align.getRotY() != 0 ||
      align.getRotZ() != 0) {
    std::cerr << "Alignment not initialized to zero" << std::endl;
    return -1;
  }

  double values[6] = { 0 };
  values[Mechanics::Alignment::OFFX] = 1;
  values[Mechanics::Alignment::OFFY] = 2;
  values[Mechanics::Alignment::OFFZ] = 3;
  values[Mechanics::Alignment::ROTX] = 4;
  values[Mechanics::Alignment::ROTY] = 5;
  values[Mechanics::Alignment::ROTZ] = 6;
  align.setAlignment(values);
  if (align.getOffX() != 1 ||
      align.getOffY() != 2 ||
      align.getOffZ() != 3 ||
      align.getRotX() != 4 ||
      align.getRotY() != 5 ||
      align.getRotZ() != 6) {
    std::cerr << "Alignment not correctly set from vector" << std::endl;
    return -1;
  }

  align.setAlignment(Mechanics::Alignment::OFFX, 10);
  align.setAlignment(Mechanics::Alignment::OFFY, 20);
  align.setAlignment(Mechanics::Alignment::OFFZ, 30);
  align.setAlignment(Mechanics::Alignment::ROTX, 40);
  align.setAlignment(Mechanics::Alignment::ROTY, 50);
  align.setAlignment(Mechanics::Alignment::ROTZ, 60);
  if (align.getOffX() != 10 ||
      align.getOffY() != 20 ||
      align.getOffZ() != 30 ||
      align.getRotX() != 40 ||
      align.getRotY() != 50 ||
      align.getRotZ() != 60) {
    std::cerr << "Alignment not correctly set from enum" << std::endl;
    return -1;
  }

  align.setOffX(1);
  align.setOffY(2);
  align.setOffZ(3);
  align.setRotX(4);
  align.setRotY(5);
  align.setRotZ(6);
  if (align.getOffX() != 1 ||
      align.getOffY() != 2 ||
      align.getOffZ() != 3 ||
      align.getRotX() != 4 ||
      align.getRotY() != 5 ||
      align.getRotZ() != 6) {
    std::cerr << "Alignment not correctly set from setters" << std::endl;
    return -1;
  }

  if (align.getOffX() != align.getAlignment(Mechanics::Alignment::OFFX) ||
      align.getOffY() != align.getAlignment(Mechanics::Alignment::OFFY) ||
      align.getOffZ() != align.getAlignment(Mechanics::Alignment::OFFZ) ||
      align.getRotX() != align.getAlignment(Mechanics::Alignment::ROTX) ||
      align.getRotY() != align.getAlignment(Mechanics::Alignment::ROTY) ||
      align.getRotZ() != align.getAlignment(Mechanics::Alignment::ROTZ)) {
    std::cerr << "getAlignment is incorrect" << std::endl;
    return -1;
  }

  return 0;
}

int test_transform() {
  const double rx = 0.1;
  const double ry = 0.2;
  const double rz = 0.3;
  const double ox = 1.1;
  const double oy = 2.2;
  const double oz = 3.3;
  const double x0 = 0.4;
  const double y0 = 0.5;
  const double z0 = 0.6;

  double matrix[3][3] = { 0 };
  matrix[0][0] = cos(ry) * cos(rz);
  matrix[0][1] = -cos(rx) * sin(rz) + sin(rx) * sin(ry) * cos(rz);
  matrix[0][2] = sin(rx) * sin(rz) + cos(rx) * sin(ry) * cos(rz);
  matrix[1][0] = cos(ry) * sin(rz);
  matrix[1][1] = cos(rx) * cos(rz) + sin(rx) * sin(ry) * sin(rz);
  matrix[1][2] = -sin(rx) * cos(rz) + cos(rx) * sin(ry) * sin(rz);
  matrix[2][0] = -sin(ry);
  matrix[2][1] = sin(rx) * cos(ry);
  matrix[2][2] = cos(rx) * cos(ry);

  Mechanics::Alignment align;
  align.setOffX(ox);
  align.setOffY(oy);
  align.setOffZ(oz);
  align.setRotX(rx);
  align.setRotY(ry);
  align.setRotZ(rz);

  double expected[3] = {
    matrix[0][0]*x0 + matrix[0][1]*y0 + matrix[0][2]*z0 + ox,
    matrix[1][0]*x0 + matrix[1][1]*y0 + matrix[1][2]*z0 + oy,
    matrix[2][0]*x0 + matrix[2][1]*y0 + matrix[2][2]*z0 + oz,
  };

  double values[3] = { x0, y0, z0 };
  align.transform(values);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Transformation failed" << std::endl;
      return -1;
    }
  }

  return 0;
}

int test_partialTransform() {
  const double rx = 0.1;
  const double ry = 0.2;
  const double rz = 0.3;
  const double ox = 1.1;
  const double oy = 2.2;
  const double oz = 3.3;
  const double x0 = 0.4;
  const double y0 = 0.5;
  const double z0 = 0.6;

  double matrix[3][3] = { 0 };
  matrix[0][0] = cos(ry) * cos(rz);
  matrix[0][1] = -cos(rx) * sin(rz) + sin(rx) * sin(ry) * cos(rz);
  matrix[0][2] = sin(rx) * sin(rz) + cos(rx) * sin(ry) * cos(rz);
  matrix[1][0] = cos(ry) * sin(rz);
  matrix[1][1] = cos(rx) * cos(rz) + sin(rx) * sin(ry) * sin(rz);
  matrix[1][2] = -sin(rx) * cos(rz) + cos(rx) * sin(ry) * sin(rz);
  matrix[2][0] = -sin(ry);
  matrix[2][1] = sin(rx) * cos(ry);
  matrix[2][2] = cos(rx) * cos(ry);

  Mechanics::Alignment align;
  align.setOffX(ox);
  align.setOffY(oy);
  align.setOffZ(oz);
  align.setRotX(rx);
  align.setRotY(ry);
  align.setRotZ(rz);

  double expected[3] = { 0 };
  double values[3] = { 0 };

  align.toggleRotation(false);
  align.toggleOffset(false);

  expected[0] = x0;
  expected[1] = y0;
  expected[2] = z0;
  values[0] = x0;
  values[1] = y0;
  values[2] = z0;
  align.transform(values);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Transformation failed with no rotation or offset" << std::endl;
      return -1;
    }
  }

  align.toggleRotation(true);
  align.toggleOffset(false);

  expected[0] = matrix[0][0]*x0 + matrix[0][1]*y0 + matrix[0][2]*z0;
  expected[1] = matrix[1][0]*x0 + matrix[1][1]*y0 + matrix[1][2]*z0;
  expected[2] = matrix[2][0]*x0 + matrix[2][1]*y0 + matrix[2][2]*z0;
  values[0] = x0;
  values[1] = y0;
  values[2] = z0;
  align.transform(values);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Transformation failed with only rotation" << std::endl;
      return -1;
    }
  }

  align.toggleOffset(true);
  align.toggleRotation(false);

  expected[0] = x0 + ox;
  expected[1] = y0 + oy;
  expected[2] = z0 + oz;
  values[0] = x0;
  values[1] = y0;
  values[2] = z0;
  align.transform(values);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Transformation failed with no rotation" << std::endl;
      return -1;
    }
  }

  return 0;
}

int test_inverseTransform() {
  const double rx = 0.1;
  const double ry = 0.2;
  const double rz = 0.3;
  const double ox = 1.1;
  const double oy = 2.2;
  const double oz = 3.3;
  const double x0 = 0.4;
  const double y0 = 0.5;
  const double z0 = 0.6;

  Mechanics::Alignment align;
  align.setOffX(ox);
  align.setOffY(oy);
  align.setOffZ(oz);
  align.setRotX(rx);
  align.setRotY(ry);
  align.setRotZ(rz);

  double expected[3] = { x0, y0, z0 };
  double values[3] = { x0, y0, z0 };

  align.transform(values);
  align.transform(values, true);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Inverse transformation failed" << std::endl;
      return -1;
    }
  }

  align.toggleRotation(false);
  align.toggleOffset(true);

  align.transform(values);
  align.transform(values, true);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Inverse transformation failed with no rotation" << std::endl;
      return -1;
    }
  }

  align.toggleRotation(true);
  align.toggleOffset(false);

  align.transform(values);
  align.transform(values, true);

  for (int i = 0; i < 3; i++) {
    if (!approxEqual(values[i], expected[i])) {
      std::cerr << "Inverse transformation failed with no translation" << std::endl;
      return -1;
    }
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_setGet()) != 0) return retval;
    if ((retval = test_transform()) != 0) return retval;
    if ((retval = test_partialTransform()) != 0) return retval;
    if ((retval = test_inverseTransform()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
