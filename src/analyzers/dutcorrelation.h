#ifndef DUTCORRELATIONS_H
#define DUTCORRELATIONS_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }

namespace Analyzers {

class DUTCorrelation : public DualAnalyzer
{
private:
  TDirectory* _corrDir;
  TDirectory* _alignDir;
  std::vector<TH2D*> _corrX;
  std::vector<TH2D*> _corrY;
  std::vector<TH1D*> _alignX;
  std::vector<TH1D*> _alignY;

  //Matevz 20141202 Alignment for pad and strip detectors
  std::vector<TH1D*> _alignPadX; // Correlation for pads in X
  std::vector<TH1D*> _alignPadY; // Correlation for pads in Y


  unsigned int _nearestRef;

  void initializeHist(const Mechanics::Sensor* sensor0,
                      const Mechanics::Sensor* sensor1);

public:
  DUTCorrelation(const Mechanics::Device* refDevice,
                 const Mechanics::Device* dutDevice,
                 TDirectory* dir = 0,
                 const char* suffix = "");

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();

  //added matevz 20141130
  TH2D* getCorrelationPlotX(unsigned int nsensor) {return _corrX.at(nsensor);};
  TH2D* getCorrelationPlotY(unsigned int nsensor) {return _corrY.at(nsensor);};

  TH1D* getAlignmentPlotX(unsigned int nsensor);
  TH1D* getAlignmentPlotY(unsigned int nsensor);

  //Matevz 20141202 getters
  TH1D* getPadAlignmentPlotX(unsigned int nsensor);
  TH1D* getPadAlignmentPlotY(unsigned int nsensor);
};

}

#endif
