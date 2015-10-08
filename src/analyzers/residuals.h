#ifndef RESIDUALS_H
#define RESIDUALS_H

#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TDirectory.h>

#include "singleanalyzer.h"

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Analyzers {

class Residuals : public SingleAnalyzer
{
private:
  std::vector<TH1D*> _residualsX;
  std::vector<TH1D*> _residualsY;  
  std::vector<TH1D*> _residualsZ;
  std::vector<TH2D*> _residualsRR;
  std::vector<TH2D*> _clusterOcc;    
  std::vector<TH2D*> _residualsXX;
  std::vector<TH2D*> _residualsXY;
  std::vector<TH2D*> _residualsYY;
  std::vector<TH2D*> _residualsYX;
  std::vector<TH2D*> _residualsXclustersize;
  std::vector<TH2D*> _residualsYclustersize;    

  double _totResidual;

public:
  Residuals(const Mechanics::Device* refDevice,
            TDirectory* dir = 0,
            const char* suffix = "",
            /* Histogram options */
            unsigned int nPixX = 20, // Number of pixels in the residual plots
            double binsPerPix = 10, // Hist bins per pixel
            unsigned int binsY = 20); // Number of bins for the vertical in AB plots

  void processEvent(const Storage::Event* refEvent);
  void postProcessing();

  double GetTotalResidual() {return _totResidual;}
  TH2D* getResidualXX(unsigned int nsensor);
  TH2D* getResidualXY(unsigned int nsensor);
  TH2D* getResidualYY(unsigned int nsensor);
  TH2D* getResidualYX(unsigned int nsensor);
};

}

#endif // RESIDUALS_H
