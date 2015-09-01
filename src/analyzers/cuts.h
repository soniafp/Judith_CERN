#ifndef CUTS_H
#define CUTS_H

#include "../storage/event.h"
#include "../storage/track.h"
#include "../storage/plane.h"
#include "../storage/cluster.h"
#include "../storage/hit.h"
#include "../mechanics/sensor.h"
#include "../processors/processors.h"

/* Loopers can have a list of generic Cuts from which checkEvent can
 * always be called. By looping over the list of cuts, there is no need to
 * check unused cuts. Furthermore, the checkEvent functions are inlined, so
 * the cuts should behave as a simple if statement in the looper code. This
 * should be very fast */

namespace Analyzers {

class Cut
{
public:
  enum Type { EQ = 1, LT = 2, GT = 3 }; // EQual, Less Than, Greater Than
protected:
  const Type _type;
public:
  Cut(Type type = EQ) : _type(type) { }
  virtual ~Cut() { }
};

class EventCut : public Cut
{
public:
  EventCut(Type type = EQ) : Cut(type) { }
  virtual ~EventCut() { }
  virtual bool check(const Storage::Event* event) const = 0;
};

class TrackCut : public Cut
{
public:
  TrackCut(Type type = EQ) : Cut(type) { }
  virtual ~TrackCut() { }
  virtual bool check(const Storage::Track* track) const = 0;
};

class ClusterCut : public Cut
{
public:
 ClusterCut(Type type = EQ, std::string name="") : Cut(type), _name(name) { }
  virtual ~ClusterCut() { }
  virtual bool check(const Storage::Cluster* cluster) const = 0;
  virtual double getSlope() { return 0.0; };
  virtual double getIntercept() { return 0.0; }
  virtual void setIntercept(const double ) {}
  virtual void setSlope(const double ) {}
  
  std::string _name;
  std::string getName(){ return _name; }

};

class HitCut : public Cut
{
public:
  HitCut(Type type = EQ) : Cut(type) { }
  virtual ~HitCut() { }
  virtual bool check(const Storage::Hit* hit) const = 0;
};

namespace Cuts {

/*******************************************************************************
 * EVENT CUTS
 */

class EventHits : public EventCut
{
private:
  const unsigned int _value;
public:
  EventHits(unsigned int value, Type type) : EventCut(type), _value(value) { }
  inline bool check(const Storage::Event* event) const
  {
    if (_type == EQ && event->getNumHits() != _value) return false;
    if (_type == LT && event->getNumHits() > _value) return false;
    if (_type == GT && event->getNumHits() < _value) return false;
    return true;
  }
};

class EventClusters : public EventCut
{
private:
  const unsigned int _value;
public:
  EventClusters(unsigned int value, Type type) : EventCut(type), _value(value) { }
  inline bool check(const Storage::Event* event) const
  {
    if (_type == EQ && event->getNumClusters() != _value) return false;
    if (_type == LT && event->getNumClusters() > _value) return false;
    if (_type == GT && event->getNumClusters() < _value) return false;
    return true;
  }
};

class EventTrigOffset : public EventCut
{
private:
  const unsigned int _value;
public:
  EventTrigOffset(unsigned int value, Type type) : EventCut(type), _value(value) { }
  inline bool check(const Storage::Event* event) const
  {
    if (_type == EQ && event->getTriggerOffset() != _value) return false;
    if (_type == LT && event->getTriggerOffset() > _value) return false;
    if (_type == GT && event->getTriggerOffset() < _value) return false;
    return true;
  }
};

class EventTracks : public EventCut
{
private:
  const unsigned int _value;
public:
  EventTracks(unsigned int value, Type type) : EventCut(type), _value(value) { }
  inline bool check(const Storage::Event* event) const
  {
    if (_type == EQ && event->getNumTracks() != _value) return false;
    if (_type == LT && event->getNumTracks() > _value) return false;
    if (_type == GT && event->getNumTracks() < _value) return false;
    return true;
  }
};

/*******************************************************************************
 * TRACK CUTS
 */

class TrackClusters : public TrackCut
{
private:
  const unsigned int _value;
public:
  TrackClusters(double value, Type type) : TrackCut(type), _value(value) { }
  inline bool check(const Storage::Track* track) const
  {
    if (_type == EQ && track->getNumClusters() != _value) return false;
    if (_type == LT && track->getNumClusters() > _value) return false;
    if (_type == GT && track->getNumClusters() < _value) return false;
    return true;
  }
};

class TrackChi2 : public TrackCut
{
private:
  const double _value;
public:
  TrackChi2(double value, Type type) : TrackCut(type), _value(value) { }
  inline bool check(const Storage::Track* track) const
  {
    if (_type == EQ && track->getChi2() != _value) return false;
    if (_type == LT && track->getChi2() > _value) return false;
    if (_type == GT && track->getChi2() < _value) return false;
    return true;
  }
};

class TrackOriginX : public TrackCut
{
private:
  const double _value;
public:
  TrackOriginX(double value, Type type) : TrackCut(type), _value(value) { }
  inline bool check(const Storage::Track* track) const
  {
    if (_type == EQ && track->getOriginX() != _value) return false;
    if (_type == LT && track->getOriginX() > _value) return false;
    if (_type == GT && track->getOriginX() < _value) return false;
    return true;
  }
};

class TrackOriginY : public TrackCut
{
private:
  const double _value;
public:
  TrackOriginY(double value, Type type) : TrackCut(type), _value(value) { }
  inline bool check(const Storage::Track* track) const
  {
    if (_type == EQ && track->getOriginY() != _value) return false;
    if (_type == LT && track->getOriginY() > _value) return false;
    if (_type == GT && track->getOriginY() < _value) return false;
    return true;
  }
};

/*******************************************************************************
 * CLUSTER CUTS
 */

class ClusterMatch : public ClusterCut
{
private:
  const unsigned int _value;
public:
  ClusterMatch(unsigned int value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    // If no match, the matching distance is always greater than anything
    if (_type == GT && !cluster->getMatchedTrack()) return true;
    // If no match, the matching distance is never smaller or equal to anything
    if (_type != GT && !cluster->getMatchedTrack()) return false;
    // If there is a match, compare to the distance
    if (_type == EQ && cluster->getMatchDistance() != _value) return false;
    if (_type == LT && cluster->getMatchDistance() > _value) return false;
    if (_type == GT && cluster->getMatchDistance() < _value) return false;
    return true;
  }
};

class ClusterToverV : public ClusterCut
{
private:
  const double _value;
  double _intercept;
public:
 ClusterToverV(double value, Type type, std::string name="") : ClusterCut(type, name), _value(value), _intercept(0.0) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    double val = _intercept + _value*cluster->getValue();
    // inclusiding the slope
    if (_type == EQ && cluster->getTiming() != val) return false;
    if (_type == LT && cluster->getTiming() > val) return false;
    if (_type == GT && cluster->getTiming() < val) return false;
    return true;
  }
  void setIntercept(const double intercept) { _intercept = intercept; }
  double getSlope() { return _value; }   
};

class ClusterT0 : public ClusterCut
{
private:
  const double _value;
public:
  ClusterT0(double value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    if (_type == EQ && cluster->getT0() != _value) return false;
    if (_type == LT && cluster->getT0() > _value) return false;
    if (_type == GT && cluster->getT0() < _value) return false;
    return true;
  }
};

class ClusterInterceptTime : public ClusterCut
{
private:
  const double _value;
  double _slope;
public:
 ClusterInterceptTime(double value, Type type, std::string name="") : ClusterCut(type, name), _value(value), _slope(-10.0) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    double val = _value + _slope*cluster->getValue();
    if (_type == EQ && cluster->getTiming() != val) return false;
    if (_type == LT && cluster->getTiming() > val) return false;
    if (_type == GT && cluster->getTiming() < val) return false;
    return true;
  } 
  void setSlope(const double slope) { _slope = slope; }
  double getIntercept() { return _value; }   
}; 

class ClusterHits : public ClusterCut
{
private:
  const unsigned int _value;
public:
  ClusterHits(unsigned int value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    if (_type == EQ && cluster->getNumHits() != _value) return false;
    if (_type == LT && cluster->getNumHits() > _value) return false;
    if (_type == GT && cluster->getNumHits() < _value) return false;
    return true;
  }
};

class ClusterValue : public ClusterCut
{
private:
  const unsigned int _value;
public:
  ClusterValue(unsigned int value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    if (_type == EQ && (unsigned int)cluster->getValue() != _value) return false;
    if (_type == LT && (unsigned int)cluster->getValue() > _value) return false;
    if (_type == GT && (unsigned int)cluster->getValue() < _value) return false;
    return true;
  }
};

class ClusterTiming : public ClusterCut
{
private:
  const unsigned int _value;
public:
  ClusterTiming(unsigned int value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    if (_type == EQ && (unsigned int)cluster->getTiming() != _value) return false;
    if (_type == LT && (unsigned int)cluster->getTiming() > _value) return false;
    if (_type == GT && (unsigned int)cluster->getTiming() < _value) return false;
    return true;
  }
};

class ClusterPosX : public ClusterCut
{
private:
  const double _value;
public:
  ClusterPosX(double value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {    
    if (_type == EQ && cluster->getPosX() != _value) return false;
    if (_type == LT && cluster->getPosX() > _value) return false;
    if (_type == GT && cluster->getPosX() < _value) return false;
    return true;
  }
};

class ClusterPosY : public ClusterCut
{
private:
  const double _value;
public:
  ClusterPosY(double value, Type type) : ClusterCut(type), _value(value) { }
  inline bool check(const Storage::Cluster* cluster) const
  {
    if (_type == EQ && cluster->getPosY() != _value) return false;
    if (_type == LT && cluster->getPosY() > _value) return false;
    if (_type == GT && cluster->getPosY() < _value) return false;
    return true;
  }
};

/*******************************************************************************
 * HIT CUTS
 */

class HitValue : public HitCut
{
private:
  const unsigned int _value;
public:
  HitValue(unsigned int value, Type type) : HitCut(type), _value(value) { }
  inline bool check(const Storage::Hit* hit) const
  {
    if (_type == EQ && (unsigned int)hit->getValue() != _value) return false;
    if (_type == LT && (unsigned int)hit->getValue() > _value) return false;
    if (_type == GT && (unsigned int)hit->getValue() < _value) return false;
    return true;
  }
};

class HitTiming : public HitCut
{
private:
  const unsigned int _value;
public:
  HitTiming(unsigned int value, Type type) : HitCut(type), _value(value) { }
  inline bool check(const Storage::Hit* hit) const
  {
    if (_type == EQ && (unsigned int)hit->getTiming() != _value) return false;
    if (_type == LT && (unsigned int)hit->getTiming() > _value) return false;
    if (_type == GT && (unsigned int)hit->getTiming() < _value) return false;
    return true;
  }
};

class HitPosX : public HitCut
{
private:
  const double _value;
public:
  HitPosX(double value, Type type) : HitCut(type), _value(value) { }
  inline bool check(const Storage::Hit* hit) const
  {
    if (_type == EQ && hit->getPosX() != _value) return false;
    if (_type == LT && hit->getPosX() > _value) return false;
    if (_type == GT && hit->getPosX() < _value) return false;
    return true;
  }
};

class HitPosY : public HitCut
{
private:
  const double _value;
public:
  HitPosY(double value, Type type) : HitCut(type), _value(value) { }
  inline bool check(const Storage::Hit* hit) const
  {
    if (_type == EQ && hit->getPosY() != _value) return false;
    if (_type == LT && hit->getPosY() > _value) return false;
    if (_type == GT && hit->getPosY() < _value) return false;
    return true;
  }
};

}

}

#endif // CUTS_H
