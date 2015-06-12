#ifndef UTILS_H
#define UTILS_H

#include <TH1.h>
#include <TF1.h>

namespace Utils {

void preFitGausBg(
    TH1& hist,
    double& mode,
    double& hwhm,
    double& norm,
    double& bg);

void fitGausBg(
    TH1& hist,
    double& mean,
    double& sigma,
    double& norm,
    double& bg,
    bool prefit=false,
    bool display=false,
    double fitRange=5);

void linearFit(
    const unsigned n,
    const double* x,
    const double* y,
    const double* ye,
    double& a,
    double& ae,
    double& b,
    double& be,
    double& chi2,
    double& cov);

}

#endif  // UTILS_H

