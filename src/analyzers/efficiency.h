#ifndef EFFICIENCY_H
#define EFFICIENCY_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TEfficiency.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class Efficiency : public DualAnalyzer
{
private:
  std::vector<TEfficiency*> _efficiencyMap;
  std::vector<TH1D*> _efficiencyDistribution;
  std::vector<TH1D*> _matchedTracks;
  std::vector<TEfficiency*> _efficiencyTime;
  std::vector<TEfficiency*> _inPixelEfficiency;
  std::vector<TEfficiency*> _inPixelEfficiencyExtended;
  std::vector<TH2D*> _inPixelCCE;
  std::vector<TH1D*> _amplDist;
  std::vector<TH1D*> _amplDistCuts;
  std::vector<TH1D*> _amplDistCommon;

   // Parameters used during event processing
  const int _relativeToSensor;

public:
  Efficiency(const Mechanics::Device* refDevice,
             const Mechanics::Device* dutDevice,
             TDirectory* dir = 0,
             const char* suffix = "",
             /* Additional parameters with default values */
             int relativeToSensor = -1, // Only consider events where this sensor has a match
             /* Histogram options */
             unsigned int rebinX = 1, // This many pixels in X are grouped
             unsigned int rebinY = 1,
             unsigned int pixBinsX = 20,
             unsigned int pixBinsY = 20);

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();
};

}

#endif // EFFICIENCY_H
