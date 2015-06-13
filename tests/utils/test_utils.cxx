#include <iostream>
#include <stdexcept>
#include <cmath>

#include <TGraphErrors.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TH1D.h>

#include "utils.h"

bool approxEqual(double v1, double v2, double tol=1E-10) {
  return std::fabs(v1-v2) < tol;
}

int test_linearFit() {
  const unsigned n = 4;
  const double x[] =  { 1, 2, 3, 4 };
  const double y[] = { .1, .21, .29, .45 };
  const double ye[] = { .01, .02, .005, 0.03 };


  TGraphErrors graph(4, x, y, 0, ye);
  TF1 f1("f1", "pol1", 0, 5);

  TFitResultPtr fit = graph.Fit(&f1, "NSQ");

  // ROOT results
  const double rp0 = fit->Parameter(0);
  const double rp0e = fit->ParError(0);
  const double rp1 = fit->Parameter(1);
  const double rp1e = fit->ParError(1);
  const double rcov = fit->GetCovarianceMatrix()[0][1];
  const double rchi2 = fit->Chi2();

  double p0, p1, p0e, p1e, cov, chi2;
  Utils::linearFit(n, x, y, ye, p0, p1, p0e, p1e, cov, chi2);

  if (!approxEqual(rp0, p0, 1E-5) ||
      !approxEqual(rp1, p1, 1E-5) ||
      !approxEqual(rp0e, p0e, 1E-5) ||
      !approxEqual(rp1e, p1e, 1E-5) ||
      !approxEqual(rcov, cov, 1E-5) ||
      !approxEqual(rchi2, chi2, 1E-5)) {
    std::cerr << "Utils: lineaFit: values don't match" << std::endl;
    return -1;
  }

  return 0;
}

int test_preFitGausBg() {
  const Int_t nbins = 21;
  TH1D hist("hist", "hist", nbins, -5, 5);

  const double width = 10./nbins;

  for (Int_t i = 1; i <= nbins; i++)
    hist.SetBinContent(i, 1);

  hist.SetBinContent(1, 10);
  hist.SetBinContent(2, 6);
  hist.SetBinContent(3, 5);
  hist.SetBinContent(4, 2);

  double mode, hwhm, norm, bg;
  Utils::preFitGausBg(hist, mode, hwhm, norm, bg);

  if (!approxEqual(mode, hist.GetBinCenter(1), 1e-5) ||
      !approxEqual(hwhm, 2*width, 1e-5) ||
      !approxEqual(norm, 10 * 2.50662827463 * hwhm, 1e-5) ||
      !approxEqual(bg, 10/2.+1/2., 1e-5)) {
    std::cerr << "Utils: preFitGausBg: left edge failed" << std::endl;
    return -1;
  }

  for (Int_t i = 1; i <= nbins; i++)
    hist.SetBinContent(i, 1);

  hist.SetBinContent(nbins, 10);
  hist.SetBinContent(nbins-1, 6);
  hist.SetBinContent(nbins-2, 5);
  hist.SetBinContent(nbins-3, 2);

  Utils::preFitGausBg(hist, mode, hwhm, norm, bg);

  if (!approxEqual(mode, hist.GetBinCenter(nbins), 1e-5) ||
      !approxEqual(hwhm, 2*width, 1e-5) ||
      !approxEqual(norm, 10 * 2.50662827463 * hwhm, 1e-5) ||
      !approxEqual(bg, 10/2.+1/2., 1e-5)) {
    std::cerr << "Utils: preFitGausBg: right edge failed" << std::endl;
    return -1;
  }

  for (Int_t i = 1; i <= nbins; i++)
    hist.SetBinContent(i, 1);

  hist.SetBinContent(nbins/2+1, 10);
  hist.SetBinContent(nbins/2+1+1, 6);
  hist.SetBinContent(nbins/2+1+2, 4);
  hist.SetBinContent(nbins/2+1-1, 4);
  hist.SetBinContent(nbins/2+1-2, 2);

  Utils::preFitGausBg(hist, mode, hwhm, norm, bg);

  if (!approxEqual(mode, hist.GetBinCenter(nbins/2+1), 1e-5) ||
      !approxEqual(hwhm, 2*width, 1e-5) ||
      !approxEqual(norm, 10 * 2.50662827463 * hwhm, 1e-5) ||
      !approxEqual(bg, 1, 1e-5)) {
    std::cerr << "Utils: preFitGausBg: central failed" << std::endl;
    return -1;
  }

  return 0;
}

int test_fitGausBg() {
  const Int_t nbins = 21;
  TH1D hist("hist", "hist", nbins, -5, 5);

  const double width = 10./nbins;

  // background per bin: 1
  // total integral of gaus: 30
  // rms of gaus: 1
  for (Int_t i = 1; i <= nbins; i++) {
    const double x = hist.GetBinCenter(i);
    hist.SetBinContent(i,
        1*width + 30*width / (1 * 2.50662827463) * std::exp(-x*x/(2*1*1)));
  }

  double mean, sigma, norm, bg;
  Utils::fitGausBg(hist, mean, sigma, norm, bg, true, false, 5);

  if (!approxEqual(mean, 0, 1e-2) ||
      !approxEqual(sigma, 1, 1e-2) ||
      !approxEqual(norm, 30, 1e-2) ||
      !approxEqual(bg, 1, 1e-3)) {
    std::cerr << "Utils: fitGausBg: closure test failed" << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  int retval = 0;

  try {
    if ((retval = test_linearFit()) != 0) return retval;
    if ((retval = test_preFitGausBg()) != 0) return retval;
    if ((retval = test_fitGausBg()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
