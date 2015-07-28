#ifndef CLUSTER_H
#define CLUSTER_H

#include <vector>

namespace Storage {

class Hit;
class Track;
class Plane;

class Cluster
{
private:
  double _pixX;
  double _pixY;
  double _pixErrX;
  double _pixErrY;
  double _posX; // Center of gravity's x position
  double _posY;
  double _posZ;
  double _posErrX; // Uncertainty of the x measurement
  double _posErrY;
  double _posErrZ;
  double _timing; // The timing of the underlying hits
  double _value; // The total value of all its hits
  double _matchDistance; // Distance to matched track

  Track* _track; // The track containing this cluster
  Track* _matchedTrack; // Track matched to this cluster in DUT analysis (not stored)

  unsigned int _numHits;
  std::vector<Hit*> _hits; // List of hits composing the cluster

protected:
  int _index;
  Plane* _plane; // The plane containing the cluster
  Cluster(); // The Event class manages the memory, not the user
  ~Cluster() { ; } // The user can't delete Cluster pointers

public:
  void print();

  void setTrack(Track* track);
  void setMatchedTrack(Track* track);
  void addHit(Hit* hit); // Add a hit to this cluster

  // The implementation is in the cpp file so that the classes can be included
  Hit* getHit(unsigned int n) const;
  Track* getTrack() const;
  Track* getMatchedTrack() const;
  Plane* getPlane() const;

  // Inline the setters and getters since they are used frequently
  inline void setPix(double x, double y) { _pixX = x; _pixY = y; }
  inline void setPos(double x, double y, double z) { _posX = x; _posY = y; _posZ = z; }
  inline void setPixErr(double x, double y) { _pixErrX = x; _pixErrY = y; }
  inline void setPosErr(double x, double y, double z) { _posErrX = x; _posErrY = y; _posErrZ = z; }
  inline void setMatchDistance(double value) { _matchDistance = value; }

  inline unsigned int getNumHits() const { return _numHits; }
  inline double getPixX() const { return _pixX; }
  inline double getPixY() const { return _pixY; }
  inline double getPixErrX() const { return _pixErrX; }
  inline double getPixErrY() const { return _pixErrY; }
  inline double getPosX() const { return _posX; }
  inline double getPosY() const { return _posY; }
  inline double getPosZ() const { return _posZ; }
  inline double getPosErrX() const { return _posErrX; }
  inline double getPosErrY() const { return _posErrY; }
  inline double getPosErrZ() const { return _posErrZ; }
  inline double getTiming() const { return _timing; }
  inline double getValue() const { return _value; }
  inline double getMatchDistance() const { return _matchDistance; }
  inline int getIndex() const { return _index; }

  friend class Plane;     // Needs to use the set plane method
  friend class Event;     // Needs access the constructor and destructor
  friend class StorageIO; // Needs access to the track index
};

}

#endif // CLUSTER_H
