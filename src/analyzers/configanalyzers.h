#ifndef CONFIGANALYZERS_H
#define CONFIGANALYZERS_H

#include <TFile.h>

class ConfigParser;
namespace Mechanics { class Device; }
namespace Loopers { class Looper; }

namespace Analyzers {

class EventCut;
class SingleAnalyzer;
class DualAnalyzer;
class Correlation;
class DutCorrelation;
class Efficiency;
class HitInfo;
class Matching;
class Occupancy;
class DUTResiduals;
class TrackInfo;

void configCorrelation(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       TFile* results);

void configDUTDepictor(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       Mechanics::Device* dutDevice);

void configDepictor(const ConfigParser& config,
                    Loopers::Looper* looper,
                    Mechanics::Device* refDevice);

void configDUTCorrelation(const ConfigParser& config,
                          Loopers::Looper* looper,
                          Mechanics::Device* refDevice,
                          Mechanics::Device* dutDevice,
                          TFile* results);

void configEfficiency(const ConfigParser& config,
                      Loopers::Looper* looper,
                      Mechanics::Device* refDevice,
                      Mechanics::Device* dutDevice,
                      TFile* results);

void configHitInfo(const ConfigParser& config,
                   Loopers::Looper* looper,
                   Mechanics::Device* refDevice,
                   TFile* results);

void configClusterInfo(const ConfigParser& config,
                       Loopers::Looper* looper,
                       Mechanics::Device* refDevice,
                       TFile* results);

void configMatching(const ConfigParser& config,
                    Loopers::Looper* looper,
                    Mechanics::Device* refDevice,
                    Mechanics::Device* dutDevice,
                    TFile* results);

void configOccupancy(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results);

void configResiduals(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     Mechanics::Device* dutDevice,
                     TFile* results);

void configResiduals(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results);

void configTrackInfo(const ConfigParser& config,
                     Loopers::Looper* looper,
                     Mechanics::Device* refDevice,
                     TFile* results);

void configLooper(const ConfigParser& config,
                  Loopers::Looper* looper,
                  Mechanics::Device* refDevice,
                  Mechanics::Device* dutDevice,
                  TFile* results);

}


#endif // CONFIGANALYZERS_H
