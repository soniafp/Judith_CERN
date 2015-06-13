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
    double& p0,
    double& p1,
    double& p0e,
    double& p1e,
    double& cov,
    double& chi2);

}

#endif  // UTILS_H

