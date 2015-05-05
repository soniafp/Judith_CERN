#ifndef HIT_H
#define HIT_H

namespace Storage {

class Cluster;
class Plane;

/**
  * Information about a digitzed hit. The hit is the basic unit of the cluster.
  * Members starting with m_pix are in pixel coordinates [0, npix), while those
  * starting with m_pos are in global coordinates (-size/2+shift, size/2+shift).
  *
  * Note that value and timing are typically integer type variables but can
  * be calibrated to a higher precision so are stored as doubles.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Hit {
protected:
  /** Cluster containing this hit */
  Cluster* m_cluster;
  /** X index of hit pixel (pixel space coordinate) */
  int m_pixX;
  int m_pixY;
  /** X position in global coordinates of hit pixel */
  double m_posX;
  double m_posY;
  double m_posZ;
  /** Value of hit pixel (e.g. time over threashold) */
  double m_value;
  /** Timming of hit pixel (e.g. level 1 accept) */
  double m_timing;
  /** Flag to determine if this hit is masked */
  bool m_masked;
  /** Plane in which hit belongs set by friend Plane class */
  Plane* m_plane;

  /** Clear values so the object can be re-used */
  void clear();

public:
  Hit();
  ~Hit() {}

  /** Print hit information to standard output */
  void print();

  inline int getPixX() const { return m_pixX; }
  inline int getPixY() const { return m_pixY; }
  inline double getPosX() const { return m_posX; }
  inline double getPosY() const { return m_posY; }
  inline double getPosZ() const { return m_posZ; }
  inline double getValue() const { return m_value; }
  inline double getTiming() const { return m_timing; }
  inline bool getMasked() const { return m_masked; }
  inline Cluster* fetchCluster() const { return m_cluster; }
  inline Plane* fetchPlane() const { return m_plane; }

  void setCluster(Cluster& cluster);
  inline void setPix(int x, int y) { m_pixX = x; m_pixY = y; }
  inline void setPos(double x, double y, double z) { m_posX = x; m_posY = y; m_posZ = z; }
  inline void setValue(double value) { m_value = value; }
  inline void setTiming(double timing) { m_timing = timing; }
  inline void setMasked(bool isMaksed) { m_masked = isMaksed; }

  friend class StorageIO;  // Sets values
  friend class Event;  // Sets values
};

}

#endif // HIT_H
