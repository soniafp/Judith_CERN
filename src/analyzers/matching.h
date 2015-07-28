#ifndef MATCHING_H
#define MATCHING_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class Matching : public DualAnalyzer
{
private:
  std::vector<TH1D*> _matchedTracks;
  std::vector<TH1D*> _matchDist;
  std::vector<TH1D*> _matchDistX;
  std::vector<TH1D*> _matchDistY;
  std::vector<TH2D*> _inPixelTracks;
  std::vector<TH2D*> _inPixelTot;

  TDirectory* _plotDir;

public:
  Matching(const Mechanics::Device* refDevice,
           const Mechanics::Device* dutDevice,
           TDirectory* dir = 0,
           const char* suffix = "",
           double pixelExtension = 3,
           /* Histogram options */
           double maxMatchingDist = 30,
           double sigmaBins = 10,
           unsigned int pixBinsX = 20,
           unsigned int pixBinsY = 20);

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();
};

}

#endif // MATCHING_H
