#include "dualanalyzer.h"

#include <vector>

#include <TDirectory.h>

#include "../storage/event.h"
#include "../mechanics/device.h"
#include "cuts.h"

namespace Analyzers {

void DualAnalyzer::validRefSensor(unsigned int nsensor)
{
  if (nsensor >= _refDevice->getNumSensors())
    throw "Analyzer: requested sensor exceeds range";
}

void DualAnalyzer::validDutSensor(unsigned int nsensor)
{
  if (nsensor >= _dutDevice->getNumSensors())
    throw "Analyzer: requested sensor exceeds range";
}

void DualAnalyzer::eventDeivceAgree(const Storage::Event* refEvent,
                                    const Storage::Event* dutEvent)
{
  if (refEvent->getNumPlanes() != _refDevice->getNumSensors() ||
      dutEvent->getNumPlanes() != _dutDevice->getNumSensors())
    throw "Analyzer: event / device plane mis-match";
}

TDirectory* DualAnalyzer::makeGetDirectory(const char* dirName)
{
  TDirectory* newDir = 0;
  if (_dir)
  {
    newDir = _dir->GetDirectory(dirName);
    if (!newDir) newDir = _dir->mkdir(dirName);
  }
  return newDir;
}

void DualAnalyzer::addCut(const EventCut* cut)
{
  _eventCuts.push_back(cut);
  _numEventCuts++;
}

void DualAnalyzer::addCut(const TrackCut* cut)
{
  _trackCuts.push_back(cut);
  _numTrackCuts++;
}

void DualAnalyzer::addCut(const ClusterCut* cut)
{
  _clusterCuts.push_back(cut);
  _numClusterCuts++;
}

void DualAnalyzer::addCut(const HitCut* cut)
{
  _hitCuts.push_back(cut);
  _numHitCuts++;
}

DualAnalyzer::DualAnalyzer(const Mechanics::Device* refDevice,
                           const Mechanics::Device* dutDevice,
                           TDirectory* dir,
                           const char* nameSuffix) :
  _refDevice(refDevice),
  _dutDevice(dutDevice),
  _dir(dir),
  _nameSuffix(nameSuffix),
  _postProcessed(false),
  _numEventCuts(0),
  _numTrackCuts(0),
  _numClusterCuts(0),
  _numHitCuts(0)
{ }

DualAnalyzer::~DualAnalyzer()
{
  for (unsigned int i = 0; i < _eventCuts.size(); i++)
    delete _eventCuts.at(i);
  for (unsigned int i = 0; i < _trackCuts.size(); i++)
    delete _trackCuts.at(i);
  for (unsigned int i = 0; i < _clusterCuts.size(); i++)
    delete _clusterCuts.at(i);
  for (unsigned int i = 0; i < _hitCuts.size(); i++)
    delete _hitCuts.at(i);
}

}
