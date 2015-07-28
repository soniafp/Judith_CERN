#ifndef CORRELATION_H
#define CORRELATION_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }

namespace Analyzers {

class Correlation : public SingleAnalyzer
{
private:
  std::vector<TH2D*> _corrX; // Correlation in X
  std::vector<TH2D*> _corrY;
  std::vector<TH1D*> _alignX; // Alignment (correlation "residual") in X
  std::vector<TH1D*> _alignY;

  // This analyzer uses two directories which need to be accessed by initializeHist
  TDirectory* _corrDir;
  TDirectory* _alignDir;

  // Shared function to initialze any correlation hist between two sensors
  void initializeHist(const Mechanics::Sensor* sensor0,
                      const Mechanics::Sensor* sensor1);

public:
  Correlation(const Mechanics::Device* device,
              TDirectory* dir,
              const char* suffix = "");

  void processEvent(const Storage::Event* event);
  void postProcessing();

  TH1D* getAlignmentPlotX(unsigned int nsensor);
  TH1D* getAlignmentPlotY(unsigned int nsensor);
};

}

#endif // CORRELATION_H
