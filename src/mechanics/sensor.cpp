#include "sensor.h"

#include <cassert>
#include <math.h>
#include <iostream>
#include <string>

#include "device.h"

using std::cout;
using std::endl;

namespace Mechanics {

void Sensor::print()
{
  cout << "\nSENSOR:\n"
       << "  Pos: " << getOffX() << " , "
                    << getOffY() << " , "
                    << getOffZ() << "\n"
       << "  Rot: " << getRotX() << " , "
                    << getRotY() << " , "
                    << getRotZ() << "\n"
       << "  Cols: " << getNumX() << "\n"
       << "  Rows: " << getNumY() << "\n"
       << "  Pitch X: " << getPitchX() << "\n"
       << "  Pitch Y: " << getPitchY() << "\n"
       << "  Depth: " << getDepth() << "\n"
       << "  X / X0: " << getXox0() << "\n"
       << "  Sensitive X: " << getSensitiveX() << "\n"
       << "  Sensitive Y: " << getSensitiveY() << endl;

  cout << "  Noisy pixels (" << _numNoisyPixels << ")" << endl;
  for (unsigned int nx = 0; nx < getNumX(); nx++)
    for (unsigned int ny = 0; ny < getNumY(); ny++)
      if (_noisyPixels[nx][ny]) cout << "    " << nx << " : " << ny << endl;

}

void Sensor::applyRotation(double& x, double& y, double& z, bool invert) const
{
  const double point[3] = {x, y, z};
  double* rotated[3] = {&x, &y, &z};

  // Use the sensor rotation matrix to transform
  for (int i = 0; i < 3; i++) {
    *(rotated[i]) = 0;
    for (int j = 0; j < 3; j++)
    {
      if (!invert) *(rotated[i]) += _rotation[i][j] * point[j];
      else         *(rotated[i]) += _unRotate[i][j] * point[j];
    }
  }
}

void Sensor::calculateRotation()
{
  const double rx = _rotX;
  const double ry = _rotY;
  const double rz = _rotZ;

  _rotation[0][0] = cos(ry) * cos(rz);
  _rotation[0][1] = -cos(rx) * sin(rz) + sin(rx) * sin(ry) * cos(rz);
  _rotation[0][2] = sin(rx) * sin(rz) + cos(rx) * sin(ry) * cos(rz);
  _rotation[1][0] = cos(ry) * sin(rz);
  _rotation[1][1] = cos(rx) * cos(rz) + sin(rx) * sin(ry) * sin(rz);
  _rotation[1][2] = -sin(rx) * cos(rz) + cos(rx) * sin(ry) * sin(rz);
  _rotation[2][0] = -sin(ry);
  _rotation[2][1] = sin(rx) * cos(ry);
  _rotation[2][2] = cos(rx) * cos(ry);

  // Transpose of a rotation matrix is its inverse
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      _unRotate[i][j] = _rotation[j][i];

  _normalX = 0, _normalY = 0, _normalZ = 1;
  applyRotation(_normalX, _normalY, _normalZ);
}

void Sensor::addNoisyPixel(unsigned int x, unsigned int y)
{
  assert(x < _numX && y < _numY && "Storage: tried to add noisy pixel outside sensor");
  _noisyPixels[x][y] = true;
  _numNoisyPixels++;
}

void Sensor::clearNoisyPixels()
{
  for (unsigned int x = 0; x < _numX; x++)
    for (unsigned int y = 0; y < _numY; y++)
      _noisyPixels[x][y] = false;
}

void Sensor::setOffX(double offset) { _offX = offset; }
void Sensor::setOffY(double offset) { _offY = offset; }
void Sensor::setOffZ(double offset) { _offZ = offset; }
void Sensor::setRotX(double rotation) { _rotX = rotation; calculateRotation(); }
void Sensor::setRotY(double rotation) { _rotY = rotation; calculateRotation(); }
void Sensor::setRotZ(double rotation) { _rotZ = rotation; calculateRotation(); }

void Sensor::rotateToGlobal(double& x, double& y, double& z) const
{
  applyRotation(x, y, z);
}

void Sensor::rotateToSensor(double& x, double& y, double& z) const
{
  applyRotation(x, y, z, true);
}

void Sensor::pixelToSpace(double pixX, double pixY,
                          double& x, double& y, double& z) const
{
  if (pixX >= _numX && pixY >= _numY)
    throw "Sensor: requested pixel out of range";

  x = 0, y = 0, z = 0;

  const double halfX = getSensitiveX() / 2.0;
  const double halfY = getSensitiveY() / 2.0;

  x = pixX * _pitchX; // To the middle of the pixel
  x -= halfX; // Origin on center of sensor

  y = pixY * _pitchY;
  y -= halfY;

  rotateToGlobal(x, y, z);

  x += getOffX();
  y += getOffY();
  z += getOffZ();
}

void Sensor::spaceToPixel(double x, double y, double z,
                          double& pixX, double& pixY) const
{
  const double halfX = getSensitiveX() / 2.0;
  const double halfY = getSensitiveY() / 2.0;

  // Subtract sensor offsets
  x -= getOffX();
  y -= getOffY();
  z -= getOffZ();

  rotateToSensor(x, y, z);

  x += halfX;
  y += halfY;

  double mapX = x / _pitchX;
  double mapY = y / _pitchY;

  pixX = mapX;
  pixY = mapY;
}

void Sensor::getGlobalOrigin(double& x, double& y, double& z) const
{
  x = getOffX();
  y = getOffY();
  z = getOffZ();
}

void Sensor::getNormalVector(double& x, double& y, double& z) const
{
  x = _normalX;
  y = _normalY;
  z = _normalZ;
}

unsigned int Sensor::getPosNumX() const {
  return (unsigned int)(getPosSensitiveX()/getPosPitchX());
}

unsigned int Sensor::getPosNumY() const {
  return (unsigned int)(getPosSensitiveY()/getPosPitchY());
}

double Sensor::getPosPitchX() const { 
  double pitch = 0;
  double x, y, z;
  // Make a vector pointing along the pixel's x-axis
  x = getPitchX();
  y = 0;
  z = 0;
  // Get it's x-projection
  rotateToGlobal(x, y, z);
  pitch += fabs(x);
  // Again with the pixel's y-axis
  x = 0;
  y = getPitchY();
  z = 0;
  rotateToGlobal(x, y, z);
  pitch += fabs(x);
  // Return the sum of the x-projection
  return pitch;
}

double Sensor::getPosPitchY() const { 
  double pitch = 0;
  double x, y, z;
  x = getPitchX();
  y = 0;
  z = 0;
  rotateToGlobal(x, y, z);
  pitch += fabs(y);
  x = 0;
  y = getPitchY();
  z = 0;
  rotateToGlobal(x, y, z);
  pitch += fabs(y);
  return pitch;
}

double Sensor::getPosSensitiveX() const {
  double size = 0;
  double x, y, z;
  x = getSensitiveX();
  y = 0;
  z = 0;
  rotateToGlobal(x, y, z);
  size += fabs(x);
  x = 0;
  y = getSensitiveY();
  z = 0;
  rotateToGlobal(x, y, z);
  size += fabs(x);
  return size;
}

double Sensor::getPosSensitiveY() const {
  double size = 0;
  double x, y, z;
  x = getSensitiveX();
  y = 0;
  z = 0;
  rotateToGlobal(x, y, z);
  size += fabs(y);
  x = 0;
  y = getSensitiveY();
  z = 0;
  rotateToGlobal(x, y, z);
  size += fabs(y);
  return size;
}

bool** Sensor::getNoiseMask() const { return _noisyPixels; }
unsigned int Sensor::getNumX() const { return _numX; }
unsigned int Sensor::getNumY() const { return _numY; }
double Sensor::getPitchX() const { return _pitchX; }
double Sensor::getPitchY() const { return _pitchY; }
double Sensor::getDepth() const { return _depth; }
double Sensor::getXox0() const { return _xox0; }
double Sensor::getOffX() const { return _offX; }
double Sensor::getOffY() const { return _offY; }
double Sensor::getOffZ() const { return _offZ; }
double Sensor::getRotX() const { return _rotX; }
double Sensor::getRotY() const { return _rotY; }
double Sensor::getRotZ() const { return _rotZ; }
double Sensor::getSensitiveX() const { return _sensitiveX; }
double Sensor::getSensitiveY() const { return _sensitiveY; }
const Device* Sensor::getDevice() const { return _device; }
  //const char* Sensor::getName() const { return _name.c_str(); }
  const std::string Sensor::getName() const { return _name; }

bool Sensor::sort(const Sensor* s1, const Sensor* s2)
{
  return (s1->getOffZ() < s2->getOffZ());
}

Sensor::Sensor(unsigned int numX, unsigned int numY, double pitchX, double pitchY,
               double depth, Device* device, std::string name, double xox0,
               double offX, double offY, double offZ,
               double rotX, double rotY, double rotZ) :
  _numX(numX), _numY(numY), _pitchX(pitchX), _pitchY(pitchY),
  _depth(depth), _device(device), _name(name), _xox0(xox0),
  _offX(offX), _offY(offY), _offZ(offZ),
  _rotX(rotX), _rotY(rotY), _rotZ(rotZ),
  _sensitiveX(pitchX * numX), _sensitiveY(pitchY * numY), _numNoisyPixels(0)
{
  assert(device && "Sensor: need to link the sensor back to a device.");

  calculateRotation();

  _noisyPixels = new bool*[_numX];
  for (unsigned int x = 0; x < _numX; x++)
  {
    bool* row = new bool[_numY];
    _noisyPixels[x] = row;
  }
  clearNoisyPixels();
}

Sensor::~Sensor()
{
  for (unsigned int x = 0; x < _numX; x++)
    delete[] _noisyPixels[x];
  delete[] _noisyPixels;
}

}
