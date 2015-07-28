#ifndef EVENT_H
#define EVENT_H

#include <vector>

#include <Rtypes.h>

namespace Processors { class TrackMaker; }

namespace Storage
{

class Hit;
class Cluster;
class Track;
class Plane;

class Event
{
private:
  ULong64_t _timeStamp;
  ULong64_t _frameNumber;
  unsigned int _triggerOffset;
  unsigned int _triggerInfo; // Dammit Andrej!
  bool _invalid;

  unsigned int _numHits;
  std::vector<Hit*> _hits;

  unsigned int _numClusters;
  std::vector<Cluster*> _clusters;

  unsigned int _numPlanes;
  std::vector<Plane*> _planes;

  unsigned int _numTracks;
  std::vector<Track*> _tracks;

protected:
  void addTrack(Track* track);

public:
  Event(unsigned int numPlanes);
  ~Event();

  void print();

  Hit* newHit(unsigned int nplane);
  Cluster* newCluster(unsigned int nplane);
  Track* newTrack();

  Hit* getHit(unsigned int n) const;
  Cluster* getCluster(unsigned int n) const;
  Plane* getPlane(unsigned int n) const;
  Track* getTrack(unsigned int n) const;

  inline void setInvalid(bool value) { _invalid = value; }
  inline void setTimeStamp(ULong64_t timeStamp) { _timeStamp = timeStamp; }
  inline void setFrameNumber(ULong64_t frameNumber) { _frameNumber = frameNumber; }
  inline void setTriggerOffset(unsigned int triggerOffset) { _triggerOffset = triggerOffset; }
  inline void setTriggerInfo(unsigned int triggerInfo) { _triggerInfo = triggerInfo; }

  inline unsigned int getNumHits() const { return _numHits; }
  inline unsigned int getNumClusters() const { return _numClusters; }
  inline unsigned int getNumPlanes() const { return _numPlanes; }
  inline unsigned int getNumTracks() const { return _numTracks; }
  inline bool getInvalid() const { return _invalid; }
  inline ULong64_t getTimeStamp() const { return _timeStamp; }
  inline ULong64_t getFrameNumber() const { return _frameNumber; }
  inline unsigned int getTriggerOffset() const { return _triggerOffset; }
  inline unsigned int getTriggerInfo() const { return _triggerInfo; }

  friend class Processors::TrackMaker;
};

}

#endif // EVENT_H
