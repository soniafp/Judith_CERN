#include "event.h"

#include <cassert>
#include <vector>
#include <iostream>

#include <Rtypes.h>

#include "hit.h"
#include "cluster.h"
#include "track.h"
#include "plane.h"

using std::cout;
using std::endl;

namespace Storage {

void Event::print()
{
  cout << "\nEVENT:\n"
       << "  Time stamp: " << getTimeStamp() << "\n"
       << "  Frame number: " << getFrameNumber() << "\n"
       << "  Trigger offset: " << getTriggerOffset() << "\n"
       << "  Invalid: " << getInvalid() << "\n"
       << "  Num planes: " << getNumPlanes() << "\n"
       << "  Num hits: " << getNumHits() << "\n"
       << "  Num clusters: " << getNumClusters() << endl;

  for (unsigned int nplane = 0; nplane < getNumPlanes(); nplane++)
    getPlane(nplane)->print();
}

Hit* Event::newHit(unsigned int nplane)
{
  Hit* hit = new Hit();
  _hits.push_back(hit);
  _planes.at(nplane)->addHit(hit);
  _numHits++;
  return hit;
}

Cluster* Event::newCluster(unsigned int nplane)
{
  Cluster* cluster = new Cluster();
  cluster->_index = _numClusters;
  _clusters.push_back(cluster);
  _planes.at(nplane)->addCluster(cluster);
  _numClusters++;
  return cluster;
}

void Event::addTrack(Track* track)
{
  track->_index = _numTracks;
  _tracks.push_back(track);
  _numTracks++;
}

Track* Event::newTrack()
{
  Track* track = new Track();
  addTrack(track);
  return track;
}

Hit* Event::getHit(unsigned int n) const
{
  assert(n < getNumHits() && "Event: hit index exceeds vector range");
  return _hits.at(n);
}

Cluster* Event::getCluster(unsigned int n) const
{
  assert(n < getNumClusters() && "Event: cluster index exceeds vector range");
  return _clusters.at(n);
}

Plane* Event::getPlane(unsigned int n) const
{
  assert(n < getNumPlanes() && "Event: plane index exceeds vector range");
  return _planes.at(n);
}

Track* Event::getTrack(unsigned int n) const
{
  assert(n < getNumTracks() && "Event: track index exceeds vector range");
  return _tracks.at(n);
}

Event::Event(unsigned int numPlanes) :
  _timeStamp(0), _frameNumber(0), _triggerOffset(0), _invalid(false),
  _numHits(0), _numClusters(0), _numPlanes(numPlanes), _numTracks(0)
{
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
  {
    Plane* plane = new Plane(nplane);
    _planes.push_back(plane);
  }
}

Event::~Event()
{
  for (unsigned int nhit = 0; nhit < _numHits; nhit++)
    delete _hits.at(nhit);
  for (unsigned int ncluster = 0; ncluster < _numClusters; ncluster++)
    delete _clusters.at(ncluster);
  for (unsigned int nplane = 0; nplane < _numPlanes; nplane++)
    delete _planes.at(nplane);
  for (unsigned int ntrack = 0; ntrack < _numTracks; ntrack++)
    delete _tracks.at(ntrack);
}

}
