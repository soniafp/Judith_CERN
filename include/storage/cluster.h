#ifndef CLUSTER_H
#define CLUSTER_H

#include <vector>

namespace Storage {

class Hit;
class Track;
class Plane;

/**
  * Collection of hits which are considered to belong to the same particle-
  * sensor interaction. Clusters can be associated to a track, and can also
  * be matched to a track which doesn't include the cluster. See hit for
  * information on naming conventions and friend class access.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Cluster {
protected:
  /** List of hits comprising this cluster */
  std::vector<Hit*> m_hits;
  /** If this cluster is part of a track, points to that track */
  Track* m_track;  
  /** Points to nearest track to this cluster */
  Track* m_matchedTrack;  
  /** Distance to the nearest track to this cluster */
  double m_matchDistance;
  /** Cluster's center of gravity in the pixel space x direction */
  double m_pixX;
  double m_pixY;
  /** Cluster size in the pixel space x direction */
  double m_pixErrX;
  double m_pixErrY;
  /** Cluster's center of gravity in the global space x direction */
  double m_posX;
  double m_posY;
  double m_posZ;
  /** Cluster size in the global space x direction */
  double m_posErrX;
  double m_posErrY;
  double m_posErrZ;
  /** Smallest timing value of hits comprising the cluster */
  double m_timing;
  /** Sum of value of hits comprising the cluster */
  double m_value;

  /** Index of this cluster in the list of all clusters. Needed to build hit-
    * cluster associations when reading from disk. */
  int m_index;
  /** Plane to which this cluster belongs set by friend Plane class */
  Plane* m_plane;
  /** Clear values so the object can be re-used */
  void clear();

public:
  Cluster();
  ~Cluster() {}

  /** Print hit information to standard output */
  void print();

  /** Add a hit to this cluster. Does bi-directional linking. */
  void addHit(Hit& hit);
  /** Compute the properties of the cluster from the provided hits */
  void compute();

  Hit& getHit(size_t n) const;
  inline size_t getNumHits() const { return m_hits.size(); }
  inline double getMatchDistance() const { return m_matchDistance; }
  inline double getPixX() const { return m_pixX; }
  inline double getPixY() const { return m_pixY; }
  inline double getPixErrX() const { return m_pixErrX; }
  inline double getPixErrY() const { return m_pixErrY; }
  inline double getPosX() const { return m_posX; }
  inline double getPosY() const { return m_posY; }
  inline double getPosZ() const { return m_posZ; }
  inline double getPosErrX() const { return m_posErrX; }
  inline double getPosErrY() const { return m_posErrY; }
  inline double getPosErrZ() const { return m_posErrZ; }
  inline double getTiming() const { return m_timing; }
  inline double getValue() const { return m_value; }
  inline Track* fetchTrack() const { return m_track; }
  inline Track* fetchMatchedTrack() const { return m_matchedTrack; }
  inline Plane* fetchPlane() const { return m_plane; }

  inline int getIndex() const { return m_index; }

  /** Set the track to which this cluster belongs */
  void setTrack(Track& track);
  inline void setPix(double x, double y) { m_pixX = x; m_pixY = y; }
  inline void setPos(double x, double y, double z) { m_posX = x; m_posY = y; m_posZ = z; }
  inline void setPixErr(double x, double y) { m_pixErrX = x; m_pixErrY = y; }
  inline void setPosErr(double x, double y, double z) { m_posErrX = x; m_posErrY = y; m_posErrZ = z; }
  inline void setTiming(double timing) { m_timing = timing; }
  inline void setValue(double value) { m_value = value; }

  friend class StorageIO;  // Sets values
  friend class Event;  // Access index
};

}

#endif // CLUSTER_H
