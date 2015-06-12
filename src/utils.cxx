#include <iostream>
#include <stdexcept>
#include <cmath>

#include <TH1.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>

#include "utils.h"

namespace Utils {

void preFitGausBg(
    TH1& hist,
    double& mode,
    double& hwhm,
    double& norm,
    double& bg) {
  const Int_t nbins = hist.GetNbinsX();
  const Int_t imax = hist.GetMaximumBin();

  // Get a guess at the normalization factor and mode
  norm = hist.GetBinContent(imax);
  mode = hist.GetBinCenter(imax);

  // Increase the width until half of maximum is reached
  Int_t width = 1;
  while (imax+width <= nbins || imax-width >= 1) {
    // Don't overshoot histogram bounds
    const Int_t up = std::min(imax+width, nbins);
    const Int_t down = std::max(imax-width, 1);
    // Allow either side to reach half max
    if (hist.GetBinContent(up) <= norm/2.) break;
    if (hist.GetBinContent(down) <= norm/2.) break;
    // Increment width for next iteration
    width += 1;
  }

  // The width gives an estimate of sigma (half width half max)
  hwhm = hist.GetBinWidth(imax)*width;

  // Also get an estimate of the background by going out to 5 sigma and looking
  // at the content
  const Int_t up = std::min(imax+5*width, nbins);
  const Int_t down = std::max(imax-5*width, 1);
  bg = hist.GetBinContent(up)/2. + hist.GetBinContent(down)/2.;
}

void fitGausBg(
    TH1& hist,
    double& mean,
    double& sigma,
    double& norm,
    double& bg,
    bool prefit,
    bool display,
    double fitRange) {
  // Gaussian distrubiton fitting function from -5 to +5 sigma
  TF1 gaus("g1", "[0]+[1]*exp(-(x-[2])*(x-[2])/(2*[3]*[3]))");

  // Guess at starting parameters if requested
  if (prefit) preFitGausBg(hist, mean, sigma, norm, bg);

  // Set the parameters to initial values
  gaus.SetParameter(0, bg);
  gaus.SetParameter(1, norm);
  gaus.SetParameter(2, mean);
  gaus.SetParameter(3, sigma);

  const double xmin = hist.GetXaxis()->GetXmin();
  const double xmax = hist.GetXaxis()->GetXmax();
  gaus.SetParLimits(0, 0, norm);
  gaus.SetParLimits(1, 0, 2*norm);
  gaus.SetParLimits(2, xmin, xmax);
  gaus.SetParLimits(3, 0, (xmax-xmin)/2.);

  gaus.SetRange(mean-fitRange*sigma, mean+fitRange*sigma);

  // N: no plotting
  // Q: quiet
  // S: store results
  // L: likelihood (instead of chi^2)
  // R: use function range
  TFitResultPtr fit = hist.Fit(&gaus, display ? "QLSR" : "NQLSR");

  mean = fit->Parameter(2);
  sigma = fit->Parameter(3);
  norm = fit->Parameter(1);
  bg = fit->Parameter(0);
}

}

