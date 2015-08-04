#include "configanalyzers.h"

#include <cassert>
#include <vector>
#include <string>

#include <TFile.h>

#include "cuts.h"
#include "singleanalyzer.h"
#include "dualanalyzer.h"
#include "dutdepiction.h"
#include "depiction.h"
#include "correlation.h"
#include "dutcorrelation.h"
#include "efficiency.h"
#include "hitinfo.h"
#include "clusterinfo.h"
#include "matching.h"
#include "occupancy.h"
#include "residuals.h"
#include "dutresiduals.h"
#include "trackinfo.h"
#include "eventinfo.h"
#include "../mechanics/device.h"
#include "../loopers/looper.h"
#include "../configparser.h"

namespace Analyzers {

void parseCut(const ConfigParser::Row* row,
              std::vector<EventCut*>& eventCuts,
              std::vector<TrackCut*>& trackCuts,
              std::vector<ClusterCut*>& clusterCuts,
              std::vector<HitCut*>& hitCuts)
{
  const double value = ConfigParser::valueToNumerical(row->value);

  std::string object = "";
  std::string variable = "";
  std::string typeName = "";

  unsigned int index = 0;
  size_t lastPos = 0;
  size_t pos = 0;
  while (pos != std::string::npos)
  {
    pos = row->key.find_first_of(' ', lastPos);
    const size_t len = (pos == std::string::npos) ?
          row->key.length() - lastPos :
          pos - lastPos;
    const std::string word = row->key.substr(lastPos, len);
    lastPos = pos + 1; // Skip the space

    // Ignore leading and multiple spaces
    if (!word.size() || !word.compare(" ")) continue;

    if (index == 0 && word.compare("cut")) throw "Analyzers: invalid cut";
    else if (index == 1) object = word;
    else if (index == 2) variable = word;
    else if (index == 3) typeName = word;
    else if (index > 3) throw "Analyzers: invalid cut parsed (too many elements)";

    index++;
  }

  Cut::Type type = Cut::EQ;
  if (!typeName.compare("equal"))
    type = Cut::EQ;
  else if (!typeName.compare("min"))
    type = Cut::GT;
  else if (!typeName.compare("max"))
    type = Cut::LT;
  else
    throw "Analyzers: invaluid cut type";

  if (!object.compare("event"))
  {
    EventCut* cut = 0;

    if (!variable.compare("hits"))
      cut = new Cuts::EventHits(value, type);
    else if (!variable.compare("clusters"))
      cut = new Cuts::EventClusters(value, type);
    else if (!variable.compare("tracks"))
      cut = new Cuts::EventTracks(value, type);
    else if (!variable.compare("trigoffset"))
      cut = new Cuts::EventTrigOffset(value, type);
    else
      throw "Analyzers: invalid event cut variable";

    eventCuts.push_back(cut);
  }
  else if (!object.compare("track"))
  {
    TrackCut* cut = 0;

    if (!variable.compare("clusters"))
      cut = new Cuts::TrackClusters(value, type);
    else if (!variable.compare("chi2"))
      cut = new Cuts::TrackChi2(value, type);
    else if (!variable.compare("originx"))
      cut = new Cuts::TrackOriginX(value, type);
    else if (!variable.compare("originy"))
      cut = new Cuts::TrackOriginY(value, type);
    else
      throw "Analyzers: invalid track cut variable";

    trackCuts.push_back(cut);
  }
  else if (!object.compare("cluster"))
  {
    ClusterCut* cut = 0;

    if (!variable.compare("hits"))
      cut = new Cuts::ClusterHits(value, type);
    else if (!variable.compare("value"))
      cut = new Cuts::ClusterValue(value, type);
    else if (!variable.compare("timing"))
      cut = new Cuts::ClusterTiming(value, type);
    else if (!variable.compare("posx"))
      cut = new Cuts::ClusterPosX(value, type);
    else if (!variable.compare("posy"))
      cut = new Cuts::ClusterPosY(value, type);
    else if (!variable.compare("matchdist"))
      cut = new Cuts::ClusterMatch(value, type);
    else
      throw "Analyzers: invalid cluster cut variable";

    clusterCuts.push_back(cut);
  }
  else if (!object.compare("hit"))
  {
    HitCut* cut = 0;

    if (!variable.compare("value"))
      cut = new Cuts::HitValue(value, type);
    else if (!variable.compare("timing"))
      cut = new Cuts::HitTiming(value, type);
    else if (!variable.compare("posx"))
      cut = new Cuts::HitPosX(value, type);
    else if (!variable.compare("posy"))
      cut = new Cuts::HitPosY(value, type);
    else
      throw "Analyzers: invalid hit cut variable";

    hitCuts.push_back(cut);
  }
  else
  {
    throw "Analyzers: invalid cut objet";
  }
}

void applyCuts(SingleAnalyzer* analyzer,
               std::vector<EventCut*>& eventCuts,
               std::vector<TrackCut*>& trackCuts,
               std::vector<ClusterCut*>& clusterCuts,
               std::vector<HitCut*>& hitCuts)
{
  for (unsigned int i = 0; i < eventCuts.size(); i++)
    analyzer->addCut(eventCuts.at(i));
  for (unsigned int i = 0; i < trackCuts.size(); i++)
    analyzer->addCut(trackCuts.at(i));
  for (unsigned int i = 0; i < clusterCuts.size(); i++)
    analyzer->addCut(clusterCuts.at(i));
  for (unsigned int i = 0; i < hitCuts.size(); i++)
    analyzer->addCut(hitCuts.at(i));
}

void applyCuts(DualAnalyzer* analyzer,
               std::vector<EventCut*>& eventCuts,
               std::vector<TrackCut*>& trackCuts,
               std::vector<ClusterCut*>& clusterCuts,
               std::vector<HitCut*>& hitCuts)
{
  for (unsigned int i = 0; i < eventCuts.size(); i++)
    analyzer->addCut(eventCuts.at(i));
  for (unsigned int i = 0; i < trackCuts.size(); i++)
    analyzer->addCut(trackCuts.at(i));
  for (unsigned int i = 0; i < clusterCuts.size(); i++)
    analyzer->addCut(clusterCuts.at(i));
  for (unsigned int i = 0; i < hitCuts.size(); i++)
    analyzer->addCut(hitCuts.at(i));
}

void configDUTDepictor(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       Mechanics::Device* dutDevice)
{
  assert(refDevice && dutDevice && "Looper: can't configure with null device");

  bool active = false;
  std::string suffix = "";
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  bool depictEvent = true;
  bool depictClusters = true;
  bool depictTracks = true;
  double zoom = 20.0;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Depictor"))
    {
      if (!active) return;
      DUTDepictor* analyzer =
          new DUTDepictor(refDevice, dutDevice, 0, suffix.c_str(),
                          depictEvent, depictClusters, depictTracks, zoom);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();

      depictEvent = true;
      depictClusters = true;
      depictTracks = true;
      zoom = 20.0;
    }

    if (row->isHeader) continue;
    if (row->header.compare("Depictor")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict event"))
      depictEvent = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict clusters"))
      depictClusters = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict tracks"))
      depictTracks = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("zoom"))
      zoom = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: dut correlation can't parse row";
  }
}

void configDepictor(const ConfigParser& config,
                    Loopers::Looper* looper,
                    Mechanics::Device* refDevice)
{
  assert(refDevice && "Looper: can't configure with null device");

  bool active = false;
  std::string suffix = "";
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  bool depictEvent = true;
  bool depictClusters = true;
  bool depictTracks = true;
  double zoom = 20.0;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Depictor"))
    {
      if (!active) return;
      Depictor* analyzer =
          new Depictor(refDevice, 0, suffix.c_str(),
                       depictEvent, depictClusters, depictTracks, zoom);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();

      depictEvent = true;
      depictClusters = true;
      depictTracks = true;
      zoom = 20.0;
    }

    if (row->isHeader) continue;
    if (row->header.compare("Depictor")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict event"))
      depictEvent = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict clusters"))
      depictClusters = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("depict tracks"))
      depictTracks = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("zoom"))
      zoom = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: dut correlation can't parse row";
  }
}

void configCorrelation(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Correlation"))
    {
      if (!active) return;
      Correlation* analyzer =
          new Correlation(refDevice, results->GetDirectory(""), suffix.c_str());
      // Will be deleted by looper
      looper->addAnalyzer(analyzer);
      // Cuts will be deleted by the analyzer
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts);

      active = false;
      suffix = "";
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Correlation")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: correlation can't parse row";
  }
}

void configDUTCorrelation(const ConfigParser& config,
                          Loopers::Looper* looper,
                          Mechanics::Device* refDevice,
                          Mechanics::Device* dutDevice,
                          TFile* results)
{
  assert(refDevice && dutDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End DUT Correlation"))
    {
      if (!active) return;
      DUTCorrelation* analyzer =
          new DUTCorrelation(refDevice, dutDevice, results->GetDirectory(""), suffix.c_str());
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("DUT Correlation")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: dut correlation can't parse row";
  }
}

void configEfficiency(const ConfigParser& config,
                      Loopers::Looper* looper,
                      Mechanics::Device* refDevice,
                      Mechanics::Device* dutDevice,
                      TFile* results)
{
  assert(refDevice && dutDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int relativeTo = -1;
  unsigned int pixGroupX = 1;
  unsigned int pixGroupY = 1;
  unsigned int pixBinsX = 20;
  unsigned int pixBinsY = 20;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Efficiency"))
    {
      if (!active) return;
      Efficiency* analyzer =
          new Efficiency(refDevice, dutDevice, results->GetDirectory(""), suffix.c_str(),
                         relativeTo, pixGroupX, pixGroupY, pixBinsX, pixBinsY);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      relativeTo = -1;
      pixGroupX = 1;
      pixGroupY = 1;
      pixBinsX = 20;
      pixBinsY = 20;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Efficiency")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("relative to"))
      relativeTo = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix group x"))
      pixGroupX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix group y"))
      pixGroupY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix bins x"))
      pixBinsX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix bins y"))
      pixBinsY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: efficiency can't parse row";
  }
}

void configHitInfo(const ConfigParser& config,
                   Loopers::Looper* looper,
                   Mechanics::Device* refDevice,
                   TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int lvl1bins = 16;
  unsigned int totBins = 16;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Hit Info"))
    {
      if (!active) return;
      HitInfo* analyzer =
          new HitInfo(refDevice, results->GetDirectory(""), suffix.c_str(),
                      lvl1bins, totBins);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      lvl1bins = 16;
      totBins = 16;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Hit Info")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("level 1 bins"))
      lvl1bins = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("tot bins"))
      totBins = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: hit info can't parse row";
  }
}

void configClusterInfo(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int totBins = 20;
  unsigned int maxSize = 5;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Cluster Info"))
    {
      if (!active) return;
      ClusterInfo* analyzer =
          new ClusterInfo(refDevice, results->GetDirectory(""), suffix.c_str(),
                          totBins, maxSize);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts);

      active = false;
      suffix = "";
      totBins = 20;
      maxSize = 5;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Cluster Info")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("tot bins"))
      totBins = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("max size"))
      maxSize = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: cluster info can't parse row";
  }
}

void configEventInfo(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int maxTracks = 10;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Event Info"))
    {
      if (!active) return;
      EventInfo* analyzer =
          new EventInfo(refDevice, results->GetDirectory(""), suffix.c_str(),
                        maxTracks);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts);

      active = false;
      suffix = "";
      maxTracks = 10;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Event Info")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("max tracks"))
      maxTracks = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: event info can't parse row";
  }
}

void configMatching(const ConfigParser& config,
                    Loopers::Looper* looper,
                    Mechanics::Device* refDevice,
                    Mechanics::Device* dutDevice,
                    TFile* results)
{
  assert(refDevice && dutDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  double maxDist = 20;
  double sigmaBins = 10;
  unsigned int pixBinsX = 25;
  unsigned int pixBinsY = 5;
  double pixelExtension = 3;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Matching"))
    {
      if (!active) return;
      Matching* analyzer =
          new Matching(refDevice, dutDevice, results->GetDirectory(""), suffix.c_str(),
                       pixelExtension, maxDist, sigmaBins, pixBinsX, pixBinsY);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      maxDist = 20;
      sigmaBins = 10;
      pixBinsX = 25;
      pixBinsY = 5;
      pixelExtension = 3;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Matching")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("max dist"))
      maxDist = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("sigma bins"))
      sigmaBins = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix bins x"))
      pixBinsX = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix bins y"))
      pixBinsY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("pix scale"))
      pixelExtension = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: matching can't parse row";
  }
}

void configOccupancy(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Occupancy"))
    {
      if (!active) return;
      Occupancy* analyzer =
          new Occupancy(refDevice, results->GetDirectory(""), suffix.c_str());
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Occupancy")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: can't parse row";
  }
}

void configResiduals(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     Mechanics::Device* dutDevice,
                     TFile* results)
{
  assert(refDevice && dutDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int numPixels = 5;
  double binsPerPixels = 10;
  int numBinsY = 15;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Residuals"))
    {
      if (!active) return;
      DUTResiduals* analyzer =
          new DUTResiduals(refDevice, dutDevice, results->GetDirectory(""), suffix.c_str(),
                           numPixels, binsPerPixels, numBinsY);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      numPixels = 5;
      binsPerPixels = 10;
      numBinsY = 15;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Residuals")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("num pixels"))
      numPixels = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("bins per pixel"))
      binsPerPixels = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("num bins y"))
      numBinsY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: residuals can't parse row";
  }
}

void configResiduals(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  unsigned int numPixels = 5;
  double binsPerPixels = 10;
  int numBinsY = 15;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Residuals"))
    {
      if (!active) return;
      Residuals* analyzer =
          new Residuals(refDevice, results->GetDirectory(""), suffix.c_str(),
                        numPixels, binsPerPixels, numBinsY);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      numPixels = 5;
      binsPerPixels = 10;
      numBinsY = 15;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Residuals")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("num pixels"))
      numPixels = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("bins per pixel"))
      binsPerPixels = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("num bins y"))
      numBinsY = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: residuals can't parse row";
  }
}

void configTrackInfo(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results)
{
  assert(refDevice && "Looper: can't configure with null device");
  if (!results) return;

  bool active = false;
  std::string suffix = "";
  double maxResolution = 1E-2;
  double maxSlope = 1E-2;
  double increaseArea = 1.2;
  std::vector<EventCut*> eventCuts;
  std::vector<TrackCut*> trackCuts;
  std::vector<ClusterCut*> clusterCuts;
  std::vector<HitCut*> hitCuts;

  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader && !row->header.compare("End Track Info"))
    {
      if (!active) return;
      TrackInfo* analyzer =
          new TrackInfo(refDevice, results->GetDirectory(""), suffix.c_str(),
                        maxResolution, maxSlope, increaseArea);
      looper->addAnalyzer(analyzer); // Will be deleted by looper
      applyCuts(analyzer, eventCuts, trackCuts, clusterCuts, hitCuts); // Cuts will be deleted by the analyzer

      active = false;
      suffix = "";
      maxResolution = 1E-2;
      maxSlope = 1E-2;
      increaseArea = 1.2;
      eventCuts.clear();
      trackCuts.clear();
      clusterCuts.clear();
      hitCuts.clear();
    }

    if (row->isHeader) continue;
    if (row->header.compare("Track Info")) continue;

    if (!row->key.compare("active"))
      active = ConfigParser::valueToLogical(row->value);
    else if (!row->key.compare("suffix"))
      suffix = row->value;
    else if (!row->key.compare("max resolution"))
      maxResolution = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("max slope"))
      maxSlope = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.compare("increase area"))
      increaseArea = ConfigParser::valueToNumerical(row->value);
    else if (!row->key.substr(0, 4).compare("cut "))
      parseCut(row, eventCuts, trackCuts, clusterCuts, hitCuts);
    else
      throw "Analyzers: track info can't parse row";
  }
}

void configLooper(const ConfigParser& config,
                  Loopers::Looper* looper,
                  Mechanics::Device* refDevice,
                  Mechanics::Device* dutDevice,
                  TFile* results)
{
  if (!dutDevice)
  {
    configDepictor(config, looper, refDevice);
    configCorrelation(config, looper, refDevice, results);
    configHitInfo(config, looper, refDevice, results);
    configClusterInfo(config, looper, refDevice, results);
    configOccupancy(config, looper, refDevice, results);
    configTrackInfo(config, looper, refDevice, results);
    configEventInfo(config, looper, refDevice, results);
    configResiduals(config, looper, refDevice, results);
  }
  else
  {
    configDUTDepictor(config, looper, refDevice, dutDevice);
    configDUTCorrelation(config, looper, refDevice, dutDevice, results);
    configResiduals(config, looper, refDevice, dutDevice, results);
    configMatching(config, looper, refDevice, dutDevice, results);
    configEfficiency(config, looper, refDevice, dutDevice, results);
  }
}

}
