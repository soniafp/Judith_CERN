#ifndef DUTRESIDUALS_H
#define DUTRESIDUALS_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "dualanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class DUTResiduals : public DualAnalyzer
{
private:
  std::vector<TH1D*> _residualsX;
  std::vector<TH1D*> _residualsY;
  std::vector<TH2D*> _residualsXX;
  std::vector<TH2D*> _residualsXY;
  std::vector<TH2D*> _residualsYY;
  std::vector<TH2D*> _residualsYX;
  std::vector<double> _totResidual;
  
public:
  DUTResiduals(const Mechanics::Device* refDevice,
               const Mechanics::Device* dutDevice,
               TDirectory* dir = 0,
               const char* suffix = "",
               /* Histogram options */
               unsigned int nPixX = 20, // Number of pixels in the residual plots
               double binsPerPix = 10, // Hist bins per pixel
               unsigned int binsY = 20); // Number of bins for the vertical in AB plots

  void processEvent(const Storage::Event* refEvent,
                    const Storage::Event* dutDevent);
  void postProcessing();

  double GetTotalResidual(unsigned int nsensor) {return _totResidual.at(nsensor);}

  TH1D* getResidualX(unsigned int nsensor);
  TH1D* getResidualY(unsigned int nsensor);  
  
  TH2D* getResidualXX(unsigned int nsensor);
  TH2D* getResidualXY(unsigned int nsensor);
  TH2D* getResidualYY(unsigned int nsensor);
  TH2D* getResidualYX(unsigned int nsensor);
};

}

#endif // DUTRESIDUALS_H
