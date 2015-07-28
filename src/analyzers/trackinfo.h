#ifndef TRACKINFO_H
#define TRACKINFO_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class TrackInfo : public SingleAnalyzer
{
private:
  TH2D* _origins;
  TH1D* _originsX;
  TH1D* _originsY;
  TH1D* _slopesX;
  TH1D* _slopesY;
  TH1D* _chi2;
  TH1D* _numClusters;
  std::vector<TH1D*> _resX;
  std::vector<TH1D*> _resY;

public:
  TrackInfo(const Mechanics::Device* device,
            TDirectory* dir,
            const char* suffix = "",
            /* Histogram options */
            double resWidth = 1E-2, // Widht of track resolution histos
            double maxSploe = 1E-2, // Maximal slope for track slope histos
            double increaseArea = 1.2); // Make origins plot larger than sensor by this factor

  void processEvent(const Storage::Event* event);
  void postProcessing();
};

}

#endif
