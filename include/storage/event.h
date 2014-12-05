#ifndef EVENT_H
#define EVENT_H

#include <vector>

#include <Rtypes.h>

namespace Storage {

class Hit;
class Cluster;
class Track;
class Plane;
class StorageIO;

/**
  * Centralized storage for all information pertaining to a single triggered
  * event. Provides structured access to the information in the form of
  * pixel hits, hit clusters, cluster tracks and sensor planes.
  *
  * Note that for the `Event` and its associated classes, member variable
  * access is provided through getters and setters so that a constant interface
  * can be maintained even if the underlying data structure changes.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Event {
private:
  // Disable copy and assignment operators
  Event(const Event&);
  Event& operator=(const Event&);

protected:
  /** The event can be owned by a storage object which cahces it and its
    * constituents */
  StorageIO* m_storage;
  
  /** Pointers to all the `Hit` objects */
  std::vector<Hit*> m_hits;
  /** Pointers to all the `Cluster` objects */
  std::vector<Cluster*> m_clusters;
  /** Pointers the `Plane` objects */
  std::vector<Plane*> m_planes;
  /** Pointers to the `Track` objects */
  std::vector<Track*> m_tracks;

  /** Timestamp at which this event was triggered */
  ULong64_t m_timeStamp;
  /** Sequential numbering of triggered events */
  ULong64_t m_frameNumber;
  /** Offset from when the event was read to its trigger time */
  int m_triggerOffset;
  /** Generic event information (flags, values ...) */
  int m_triggerInfo;
  /** Flag indicates if the event is corrupted or unusuable */
  bool m_invalid;

  /** Constructor managed by friend StorageIO class */
  Event(StorageIO& storage);
  /** Clear the values so the object can be re-used. Loose owernship over
    * hits, clusters and tracks */
  void clear();

public:
  /** Public constructor does not give owernship to a `StorageIO` object */
  Event(size_t numPlanes);
  /** Destructor frees memory owned by this object */
  ~Event();

  /** Print hit information to standard output */
  void print();

  /** Make a new hit for plane `nplane` owned by this event */
  Hit& newHit(size_t nplane);
  /** Make a new cluster for plane `nplane` owned by this event */
  Cluster& newCluster(size_t nplane);
  /** Make a new track owned by this event */
  Track& newTrack();

  Hit& getHit(size_t n) const;
  Cluster& getCluster(size_t n) const;
  Plane& getPlane(size_t n) const;
  Track& getTrack(size_t n) const;

  inline void setInvalid(bool value) { m_invalid = value; }
  inline void setTimeStamp(ULong64_t timeStamp) { m_timeStamp = timeStamp; }
  inline void setFrameNumber(ULong64_t frameNumber) { m_frameNumber = frameNumber; }
  inline void setTriggerOffset(int triggerOffset) { m_triggerOffset = triggerOffset; }
  inline void setTriggerInfo(int triggerInfo) { m_triggerInfo = triggerInfo; }

  inline size_t getNumHits() const { return m_hits.size(); }
  inline size_t getNumClusters() const { return m_clusters.size(); }
  inline size_t getNumPlanes() const { return m_planes.size(); }
  inline size_t getNumTracks() const { return m_tracks.size(); }
  inline ULong64_t getTimeStamp() const { return m_timeStamp; }
  inline ULong64_t getFrameNumber() const { return m_frameNumber; }
  inline int getTriggerOffset() const { return m_triggerOffset; }
  inline int getTriggerInfo() const { return m_triggerInfo; }
  inline bool getInvalid() const { return m_invalid; }

  friend StorageIO;  // Manages cache
};

}

#endif // EVENT_H
