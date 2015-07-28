#ifndef HITINFO_H
#define HITINFO_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class HitInfo : public SingleAnalyzer
{
private:
  std::vector<TH1D*> _lvl1;
  std::vector<TH1D*> _tot;
  std::vector<TH2D*> _totMap;
  std::vector<TH2D*> _totMapCnt;

  unsigned int _lvl1Bins;
  unsigned int _totBins;

public:
  HitInfo(const Mechanics::Device* device,
          TDirectory* dir,
          const char* suffix = "",
          /* Histogram options */
          unsigned int lvl1Bins = 16,
          unsigned int totBins = 16);

  void processEvent(const Storage::Event* event);
  void postProcessing();
};

}

#endif // HITINFO_H
