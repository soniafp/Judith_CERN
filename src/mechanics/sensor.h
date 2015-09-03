#ifndef SENSOR_H
#define SENSOR_H

#include <vector>
#include <string>

namespace Mechanics {

class Device;

class Sensor
{
private:
  const unsigned int _numX;
  const unsigned int _numY;
  const double _pitchX;
  const double _pitchY;
  const double _depth;
  const Device* _device;
  std::string _name;
  const double _xox0;
  double _offX;
  double _offY;
  double _offZ;
  double _rotX;
  double _rotY;
  double _rotZ;
  const double _sensitiveX;
  const double _sensitiveY;
  unsigned int _numNoisyPixels;
  bool** _noisyPixels;

  double _rotation[3][3]; // The rotation matrix for the plane
  double _unRotate[3][3]; // Invert the rotation
  double _normalX;
  double _normalY;
  double _normalZ;

  void calculateRotation(); // Calculates the rotation matricies and normal vector
  void applyRotation(double& x, double& y, double& z, bool invert = false) const;

public:
  Sensor(unsigned int numX, unsigned int numY, double pitchX, double pitchY,
         double depth, Device* device, std::string name, double xox0 = 0,
         double offX = 0, double offY = 0, double offZ = 0,
         double rotX = 0, double rotY = 0, double rotZ = 0);
  ~Sensor();

  void print();

  void addNoisyPixel(unsigned int x, unsigned int y);
  void clearNoisyPixels();
  inline bool isPixelNoisy(unsigned int x, unsigned int y) const
      { return _noisyPixels[x][y]; }

  void setOffX(double offset);
  void setOffY(double offset);
  void setOffZ(double offset);
  void setRotX(double rotation);
  void setRotY(double rotation);
  void setRotZ(double rotation);

  void rotateToGlobal(double& x, double& y, double& z) const;
  void rotateToSensor(double& x, double& y, double& z) const;
  void pixelToSpace(double pixX, double pixY,
                    double& x, double& y, double& z) const;
  void spaceToPixel(double x, double y, double z,
                    double& pixX, double& pixY) const;

  void getGlobalOrigin(double& x, double& y, double& z) const;
  void getNormalVector(double& x, double& y, double& z) const;

  bool** getNoiseMask() const;
  unsigned int getNumX() const;
  unsigned int getNumY() const;
  unsigned int getPosNumX() const;
  unsigned int getPosNumY() const;
  double getPitchX() const;
  double getPitchY() const;
  double getPosPitchX() const;
  double getPosPitchY() const;
  double getDepth() const;
  double getXox0() const;
  double getOffX() const;
  double getOffY() const;
  double getOffZ() const;
  double getRotX() const;
  double getRotY() const;
  double getRotZ() const;
  double getSensitiveX() const;
  double getSensitiveY() const;
  double getPosSensitiveX() const;
  double getPosSensitiveY() const;
  const Device* getDevice() const;
  //const char* getName() const;
  const std::string getName() const;  

  static bool sort(const Sensor* s1, const Sensor* s2);
};

}

#endif // SENSOR_H
