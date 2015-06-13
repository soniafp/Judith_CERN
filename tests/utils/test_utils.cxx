#include <iostream>
#include <stdexcept>
#include <cmath>

#include <TGraphErrors.h>
#include <TF1.h>
#include <TFitResult.h>
#include <TFitResultPtr.h>
#include <TMatrixTSym.h>

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

int main() {
  int retval = 0;

  try {
    if ((retval = test_linearFit()) != 0) return retval;
  }
  
  catch (std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
