#ifndef HIT_H
#define HIT_H

namespace Storage {

class Cluster;
class Plane;

class Hit
{
private:
  unsigned int _pixX; // X pixel of this hit
  unsigned int _pixY;
  double _posX; // X position of the hit on the sensor
  double _posY;
  double _posZ;
  double _value; // Time over threshold, typically
  int _valueInt; // Time over threshold, typically
  double _timing; // Level 1 accept, typically
  double _t0; // start of hit detection

  Cluster* _cluster; // The cluster containing this hit

protected:
  Plane* _plane; // Plane in which the hit is found
  Hit(); // Hits memory is managed by the event class
  ~Hit() { ; } // The user can get Hit pointers but can't delete them

public:
  void print();

  void setCluster(Cluster* cluster);

  // These are in the cpp file so that the classes can be included
  Cluster* getCluster() const;
  Plane* getPlane() const;

  // Inline setters and getters since they will be used frequently
  inline void setPix(unsigned int x, unsigned int y) { _pixX = x; _pixY = y; }
  inline void setPos(double x, double y, double z) { _posX = x; _posY = y; _posZ = z; }
  inline void setValue(double value) { _value = value; }
  inline void setValueInt(int value) { _valueInt = value; }
  inline void setTiming(double timing) { _timing = timing; }
  inline void setT0(double t0) { _t0 = t0; }  

  inline unsigned int getPixX() const { return _pixX; }
  inline unsigned int getPixY() const { return _pixY; }
  inline double getPosX() const { return _posX; }
  inline double getPosY() const { return _posY; }
  inline double getPosZ() const { return _posZ; }
  inline double getValue() const { return _value; }
  inline int getValueInt() const { return _valueInt; }
  inline double getTiming() const { return _timing; }
  inline double getT0() const { return _t0; }  

  friend class Plane;     // Access set plane method
  friend class Event;     // Access cluster index
  friend class StorageIO; // Constructor and destructor
};

}

#endif // HIT_H
