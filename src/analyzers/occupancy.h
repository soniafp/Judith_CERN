#ifndef OCCUPANCY_H_
#define OCCUPANCY_H_

#include <vector>

#include <Rtypes.h>
#include <TH2D.h>
#include <TH2D.h>
#include <TNtuple.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class Occupancy : public SingleAnalyzer
{
private:
  std::vector<TH2D*> _hitOcc;
  std::vector<TH2D*> _clusterOcc;
  std::vector<TNtuple*> _clusterNtuple;
  TH1D* _occDistribution;

public:
  ULong64_t totalHitOccupancy;

  Occupancy(const Mechanics::Device* device,
            TDirectory* dir,
            const char* suffix = "");

  void processEvent(const Storage::Event* event);
  void postProcessing();

  TH1D* getOccDistribution();
  TH2D* getHitOcc(unsigned int nsensor);
};

}

#endif // HITOCCUPANCY_H
