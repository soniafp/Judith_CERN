#include "processors.h"

#include <cassert>
#include <math.h>
#include <float.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>

#include <TH2D.h>
#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TSystem.h>

#include "../storage/track.h"
#include "../storage/hit.h"
#include "../storage/cluster.h"
#include "../storage/event.h"
#include "../storage/plane.h"
#include "../mechanics/sensor.h"
#include "../mechanics/device.h"
#include "../processors/trackmaker.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

using std::cout;
using std::endl;

namespace Processors {

TF1* fitPixelBeam(TH1D* hist, double pixelWidth, double beamSigma, bool display)
{
  TF1* conv = new TF1("conv",
                      "[2] + [4] * 0.5 * ( TMath::Erf(([0]/2 + [3] - x)/(sqrt(2)*[1])) + TMath::Erf(([0]/2 + [3] + x)/(sqrt(2)*[1])) )");
  conv->SetParNames("Width", "Sigma", "Background", "Offset", "Scale");

  const unsigned int numBins = hist->GetNbinsX();
  const double limitLow = hist->GetXaxis()->GetBinLowEdge(1);
  const double limitHigh = hist->GetXaxis()->GetBinUpEdge(numBins);

  // Estimate the background using the +/- 10% edges of the histogram
  double background = 0;
  const unsigned int numBackgroundBins = numBins * 0.1 + 1;
  for (unsigned int n = 0; n < numBackgroundBins; n++)
  {
    background += hist->GetBinContent(n + 1);
    background += hist->GetBinContent(numBins - n);
  }
  background /= (double)(2 * numBackgroundBins);

  // Estimate the scale
  double scale = 0;
  for (unsigned int bin = 1; bin <= numBins; bin++)
    if (hist->GetBinContent(bin) > scale) scale = hist->GetBinContent(bin);

  double offset = 0;

  conv->SetRange(limitLow, limitHigh);
  conv->SetParameters(pixelWidth, beamSigma, background, offset, scale);
  conv->SetParLimits(0, 0.1 * pixelWidth, 100 * pixelWidth);
  conv->SetParLimits(1, 0, 100 * beamSigma);
  conv->SetParLimits(2, 0, scale);
  conv->SetParLimits(3, -pixelWidth, pixelWidth);
  conv->SetParLimits(4, 0.1 * scale, 10 * scale);
  hist->Fit("conv", "QR0B");

  if (display)
  {
    TCanvas* can = new TCanvas();
    conv->SetLineColor(46);
    conv->SetLineWidth(1);
    hist->Draw();
    conv->Draw("SAME");
    can->Update();
    can->WaitPrimitive();
  }

  return conv;
}

void fitGaussian(
    TH1D* hist,
    double& mean,
    double& sigma,
    bool display) {
  double max = 0;
  double background = 0;
  fitGaussian(hist, mean, sigma, max, background, display);
}

void fitGaussian(
    TH1D* hist,
    double& mean,
    double& sigma,
    double& max,
    double& background,
    bool display)
{
  TF1* gauss = new TF1("g1", "[0]*exp(-0.5 * ((x-[1])/[2])**2) + [3]");
  gauss->SetParNames("Constant", "Mean", "Sigma", "Offset");

  int maxPos = 1;
  max = 0;
  int risePos = 1;
  double rise = 0;
  int fallPos = 1;
  double fall = 0;

  for (int bin = 1; bin <= hist->GetNbinsX(); bin++)
  {
    const double value = hist->GetBinContent(bin);
    if (value > max)
    {
      max = value;
      maxPos = bin;
    }

    const double valuePrev = hist->GetBinContent(bin > 1 ? bin-1 : bin);
    if (value - valuePrev > rise) {
      rise = value - valuePrev;
      risePos = bin;
    }

    const double valueNext = hist->GetBinContent(
        bin < hist->GetNbinsX() ? bin+1 : bin);
    if (value - valueNext > fall) {
      fall = value - valueNext;
      fallPos = bin;
    }
  }

  mean = hist->GetXaxis()->GetBinCenter(maxPos);
  sigma = hist->GetXaxis()->GetBinUpEdge(fallPos) -
      hist->GetXaxis()->GetBinLowEdge(risePos);

  const unsigned int initialWidth = 5;

  Int_t bgBin1 = hist->FindBin(mean - initialWidth * sigma);
  Int_t bgBin2 = hist->FindBin(mean + initialWidth * sigma);
  if (bgBin1 < 1) bgBin1 = 1;
  if (bgBin2 > hist->GetNbinsX()) bgBin2 = hist->GetNbinsX();
  background = hist->GetBinContent(bgBin1) + hist->GetBinContent(bgBin2);
  background /= 2.0;

  max -= background;

  // If sigma is very small, try to calculate using FWHM
//  if (sigma < 10 * DBL_MIN)
//  {
//    double pt1 = 0;
//    for (Int_t bin = 1; bin <= hist->GetNbinsX(); bin++)
//    {
//      if (bin >= max / 2.0)
//      {
//        pt1 = hist->GetBinCenter(bin);
//        break;
//      }
//    }

//    double pt2 = 0;
//    for (Int_t bin = hist->GetNbinsX(); bin >= 1; bin--)
//    {
//      if (bin >= max / 2.0)
//      {
//        pt2 = hist->GetBinCenter(bin);
//        break;
//      }
//    }

//    sigma = pt2 - pt1;
//  }

  gauss->SetRange(mean - initialWidth * sigma, mean + initialWidth * sigma);
  gauss->SetParameters(max, mean, sigma, background);
  gauss->SetParLimits(2, 0, 10 * sigma);
  hist->Fit("g1", "QR0");

  max = gauss->GetParameter(0);
  mean = gauss->GetParameter(1);
  sigma = gauss->GetParameter(2);
  background = gauss->GetParameter(3);

  gauss->SetRange(mean - 5 * sigma, mean + 5 * sigma);
  gauss->SetParameters(max, mean, sigma, background);
  hist->Fit("g1", "QR0");

  max = gauss->GetParameter(0);
  mean = gauss->GetParameter(1);
  sigma = gauss->GetParameter(2);
  background = gauss->GetParameter(3);

  if (display)
  {
    TCanvas* can = new TCanvas();
    gauss->SetLineColor(46);
    gauss->SetLineWidth(1);
    hist->Draw();
    gauss->Draw("SAME");
    can->Update();
    can->WaitPrimitive();
  }

  delete gauss;
}


void fitBox(
    TH1D* hist,
    double& mean,
    double& sigma,
    double sensorWidth,
    bool display) {
  double max = 0;
  double background = 0;
  fitBox(hist, mean, sigma, max, background, sensorWidth, display);
}


void fitBox(
    TH1D* hist,
    double& mean,
    double& sigma,
    double& max,
    double& background,
    double sensorWidth,
    bool display)
{

  TF1 *errorfunc = new TF1("errorfunc","[0]*(1+TMath::Erf([1]*([2]-x)))*(1-TMath::Erf([3]*([4]-x)))");
  errorfunc->SetParNames("Height", "FallSlope","Fall", "RiseSlope", "Rise");

  //fit a gaussian for first approximation
  fitGaussian(hist, mean, sigma, max, background, display);


  //first order approximations
  // double maxBin = hist->GetMaximumBin();
  double rise = mean - sensorWidth/2;
  double fall = mean + sensorWidth/2;
  double height = max/4;
  double riseSlope = height*10;
  double fallSlope = height*10;
  // double maxBin = 0;
  // double rise = -1000;
  // double fall = 3000;
  // double maxHeight = 40;
  // double riseSlope = 400;
  // double fallSlope = 400;


  errorfunc->SetParameter("Height",height);
  errorfunc->SetParameter("Rise",rise);
  errorfunc->SetParameter("Fall",fall);
  errorfunc->SetParameter("RiseSlope",riseSlope);
  errorfunc->SetParameter("FallSlope",fallSlope);
  errorfunc->SetParLimits(0, height-abs(0.5*height), height+abs(0.5*height));
  errorfunc->SetParLimits(4, rise-abs(1*rise), rise+abs(1*rise));
  errorfunc->SetParLimits(2, fall-abs(1*fall), fall+abs(1*fall));
  errorfunc->SetParLimits(3, 0.01*riseSlope, 100*riseSlope);
  errorfunc->SetParLimits(1, 0.01*fallSlope, 100*fallSlope);


  hist->Fit("errorfunc", "");

  //max = gauss->GetParameter(0);
  mean = ( errorfunc->GetParameter(2) + errorfunc->GetParameter(4) ) / 2 ;
  sigma = abs ( errorfunc->GetParameter(4) - errorfunc->GetParameter(2) );
  //background = gauss->GetParameter(3);

  if (display)
  {
    TCanvas* can = new TCanvas();
    errorfunc->SetLineColor(46);
    errorfunc->SetLineWidth(1);
    hist->Draw();
    errorfunc->Draw("SAME");
    can->Update();
    can->WaitPrimitive();
  }

  delete errorfunc;

//   int maxPos = 1;
//   max = 0;
//   int risePos = 1;
//   double rise = 0;
//   int fallPos = 1;
//   double fall = 0;
//
//   for (int bin = 1; bin <= hist->GetNbinsX(); bin++)
//   {
//     const double value = hist->GetBinContent(bin);
//     if (value > max)
//     {
//       max = value;
//       maxPos = bin;
//     }
//
//     const double valuePrev = hist->GetBinContent(bin > 1 ? bin-1 : bin);
//     if (value - valuePrev > rise) {
//       rise = value - valuePrev;
//       risePos = bin;
//     }
//
//     const double valueNext = hist->GetBinContent(
//         bin < hist->GetNbinsX() ? bin+1 : bin);
//     if (value - valueNext > fall) {
//       fall = value - valueNext;
//       fallPos = bin;
//     }
//   }
//
//   mean = hist->GetXaxis()->GetBinCenter(maxPos);
//   sigma = hist->GetXaxis()->GetBinUpEdge(fallPos) -
//       hist->GetXaxis()->GetBinLowEdge(risePos);
//
//   const unsigned int initialWidth = 5;
//
//   Int_t bgBin1 = hist->FindBin(mean - initialWidth * sigma);
//   Int_t bgBin2 = hist->FindBin(mean + initialWidth * sigma);
//   if (bgBin1 < 1) bgBin1 = 1;
//   if (bgBin2 > hist->GetNbinsX()) bgBin2 = hist->GetNbinsX();
//   background = hist->GetBinContent(bgBin1) + hist->GetBinContent(bgBin2);
//   background /= 2.0;
//
//   max -= background;
//
//   // If sigma is very small, try to calculate using FWHM
// //  if (sigma < 10 * DBL_MIN)
// //  {
// //    double pt1 = 0;
// //    for (Int_t bin = 1; bin <= hist->GetNbinsX(); bin++)
// //    {
// //      if (bin >= max / 2.0)
// //      {
// //        pt1 = hist->GetBinCenter(bin);
// //        break;
// //      }
// //    }
//
// //    double pt2 = 0;
// //    for (Int_t bin = hist->GetNbinsX(); bin >= 1; bin--)
// //    {
// //      if (bin >= max / 2.0)
// //      {
// //        pt2 = hist->GetBinCenter(bin);
// //        break;
// //      }
// //    }
//
// //    sigma = pt2 - pt1;
// //  }
//
//   gauss->SetRange(mean - initialWidth * sigma, mean + initialWidth * sigma);
//   gauss->SetParameters(max, mean, sigma, background);
//   gauss->SetParLimits(2, 0, 10 * sigma);
//   hist->Fit("g1", "QR0");
//
//   max = gauss->GetParameter(0);
//   mean = gauss->GetParameter(1);
//   sigma = gauss->GetParameter(2);
//   background = gauss->GetParameter(3);
//
//   gauss->SetRange(mean - 5 * sigma, mean + 5 * sigma);
//   gauss->SetParameters(max, mean, sigma, background);
//   hist->Fit("g1", "QR0");
//
//   max = gauss->GetParameter(0);
//   mean = gauss->GetParameter(1);
//   sigma = gauss->GetParameter(2);
//   background = gauss->GetParameter(3);
//
//   if (display)
//   {
//     TCanvas* can = new TCanvas();
//     gauss->SetLineColor(46);
//     gauss->SetLineWidth(1);
//     hist->Draw();
//     gauss->Draw("SAME");
//     can->Update();
//     can->WaitPrimitive();
//   }
//
//   delete gauss;
}



void residualAlignment(TH2D* residualX, TH2D* residualY, double& offsetX,
                       double& offsetY, double& rotation,
                       double relaxation, bool display)
{
  assert(residualX && residualY && "Processors: can't perform residual alignment without histograms");

  rotation = 0;
  offsetX = 0;
  offsetY = 0;
  double angleWeights = 0;
  double fitChi2 = 0;

  for (int axis = 0; axis < 2; axis++)
  {
    TH2D* hist = 0;
    if (axis) hist = residualX;
    else      hist = residualY;

    // Project the histogram and fit with a gaussian to center the sensor
//    TH1D* project = hist->ProjectionX("ResidualProjetion", 1, hist->GetNbinsY());
//    project->SetDirectory(0);

//    double sigma = project->GetBinWidth(1);
//    double mean = 0;
//    fitGaussian(project, mean, sigma, false);

//    if (axis) offsetX = mean;
//    else      offsetY = mean;

//    delete project;

    std::vector<double> ptsX;
    std::vector<double> ptsY;
    std::vector<double> ptsErr;

    const unsigned int numSlices = hist->GetNbinsY();

    for (Int_t row = 1; row <= (int)numSlices; row++)
    {
      TH1D* slice = hist->ProjectionX("ResidualSlice", row, row);
      slice->SetDirectory(0);

      double mean = 0;
      double sigma = 0;
      double factor = 0;
      double background = 0;

      if (slice->Integral() < 1) { delete slice; continue; }
      fitGaussian(slice, mean, sigma, factor, background, false);

      const double sliceMin = slice->GetBinCenter(1);
      const double sliceMax = slice->GetBinCenter(slice->GetNbinsX());
      delete slice;

      // Quality assurance

      // Sigma is contained in the slice's range
      if (sigma > (sliceMax - sliceMin)) continue;
      // Mean is contained in the slice's range
      if (mean > sliceMax || mean < sliceMin) continue;
      // Peak is contains sufficient events
      if (factor < 100) continue;
      // Sufficient signal to noise ratio
      if (factor / background < 10) continue;

      // Get the total number of events in the gaussian 1 sigma
      Int_t sigRangeLow = hist->FindBin(mean - sigma);
      Int_t sigRangeHigh = hist->FindBin(mean + sigma);

      double sigRangeTotal = 0;
      for (Int_t bin = sigRangeLow; bin <= sigRangeHigh; bin++)
        sigRangeTotal += hist->GetBinContent(bin);

      // 2 * 1 sigma integral shoudl give ~ area under gaussian
      sigma /= sqrt(2 * sigRangeTotal);

      ptsX.push_back(hist->GetYaxis()->GetBinCenter(row));
      ptsY.push_back(mean);
      ptsErr.push_back(sigma);
    }

    if (ptsX.size() < 3) continue;

    std::vector<double> yvals = ptsY;
    std::sort(yvals.begin(), yvals.end());
    const double median = yvals[yvals.size()/2];
    double avgDeviation = 0;
    for (unsigned int i = 0; i < yvals.size(); i++)
      avgDeviation += fabs(yvals[i] - median);
    avgDeviation /= (double)yvals.size();

    std::vector<double> ptsXGood;
    std::vector<double> ptsYGood;
    std::vector<double> ptsErrGood;

    for (unsigned int i = 0; i < ptsX.size(); i++)
    {
      if (fabs(ptsY[i] - median) > 1.5*avgDeviation) continue;
      ptsXGood.push_back(ptsX[i]);
      ptsYGood.push_back(ptsY[i]);
      ptsErrGood.push_back(ptsErr[i]);
    }

    if (ptsXGood.size() < 3) continue;

    TGraphErrors* graph = new TGraphErrors(ptsXGood.size(),
                                           &(ptsXGood.at(0)),
                                           &(ptsYGood.at(0)), 0,
                                           &(ptsErrGood.at(0)));

    TF1* fitFunc = new TF1("f1", "1 ++ x");
    TF1* result = 0;

    graph->Fit(fitFunc, "Q0E").Get();
    result = graph->GetFunction(fitFunc->GetName());

    // Weight the angle by the slope uncertainty and the inverse of the chi2 normalized
    double weight = result->GetParError(1);
    const double chi2 = result->GetChisquare() / (double)result->GetNDF();
    fitChi2 += chi2;
    weight *= chi2;
    if (weight > 10 * DBL_MIN) weight = 1.0 / weight;
    else weight = 1.0;

    if (axis)
    {
      rotation -= weight * atan(result->GetParameter(1));
      offsetX = result->GetParameter(0);
    }
    else
    {
      rotation += weight * atan(result->GetParameter(1));
      offsetY = result->GetParameter(0);
    }

    angleWeights += weight;

    if (display)
    {
      TCanvas* can = new TCanvas("ResidualAlignment", "Residual Alignment", 900, 600);
      can->Divide(2);
      can->cd(1);
      hist->Draw("COLZ");
      can->cd(2);
      result->SetLineColor(46);
      result->SetLineWidth(2);
      graph->Draw("ap");
      result->Draw("SAME");
      can->Update();
      can->WaitPrimitive();
    }

    delete fitFunc;
    delete graph;
  }

  if (angleWeights > 10 * DBL_MIN)
    rotation /= angleWeights;

  rotation *= relaxation;
  offsetX *= relaxation;
  offsetY *= relaxation;
}

void applyAlignment(Storage::Event* event, const Mechanics::Device* device)
{
  assert(event && device && "Processors: can't apply alignmet with null event and/or device");
  assert(event->getNumPlanes() == device->getNumSensors() &&
         "Processors: plane / sensor mismatch");

  for (unsigned int nplane = 0; nplane < event->getNumPlanes(); nplane++)
  {
    Storage::Plane* plane = event->getPlane(nplane);
    Mechanics::Sensor* sensor = device->getSensor(nplane);

    // Apply alignment to hits
    for (unsigned int nhit = 0; nhit < plane->getNumHits(); nhit++)
    {
      Storage::Hit* hit = plane->getHit(nhit);
      double posX = 0, posY = 0, posZ = 0;
      sensor->pixelToSpace(hit->getPixX() + 0.5, hit->getPixY() + 0.5, posX, posY, posZ);
      hit->setPos(posX, posY, posZ);
    }

    // Apply alignment to clusters
    for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++)
    {
      Storage::Cluster* cluster = plane->getCluster(ncluster);
      double posX = 0, posY = 0, posZ = 0;
      sensor->pixelToSpace(cluster->getPixX(), cluster->getPixY(), posX, posY, posZ);
      cluster->setPos(posX, posY, posZ);
      double errX = sensor->getPitchX() * cluster->getPixErrX();
      double errY = sensor->getPitchY() * cluster->getPixErrY();
      double errZ = 0;
      sensor->rotateToGlobal(errX, errY, errZ);
      errX = fabs(errX);
      errY = fabs(errY);
      errZ = fabs(errZ);
      cluster->setPosErr(errX, errY, errZ);
    }
  }

  // Apply alignment to tracks
  for (unsigned int ntrack = 0; ntrack < event->getNumTracks(); ntrack++)
  {
    Storage::Track* track = event->getTrack(ntrack);
    Processors::TrackMaker::fitTrackToClusters(track);
  }
}

void pixelToSlope(const Mechanics::Device* device, double &slopeX, double &slopeY)
{
  assert(device && "Processors: can't use a null device to calculate a pixel slope");

  assert(device->getNumSensors() > 1 &&
         "Processor: can't get a pixel slope with less than 2 sensors");

  const double z0 = device->getSensor(0)->getOffZ();
  const double zf = device->getSensor(device->getNumSensors() - 1)->getOffZ();

  assert(zf > z0 &&
         "Processors: sensors are in the wrong order");

  const double pixX = device->getSensor(device->getNumSensors() - 1)->getPitchX();
  const double pixY = device->getSensor(device->getNumSensors() - 1)->getPitchY();

  slopeX = pixX / (zf - z0);
  slopeY = pixY / (zf - z0);
}

int lineSensorIntercept(double posX, double posY, double posZ,
                        double slopeX, double slopeY,
                        const Mechanics::Sensor* sensor,
                        double& x, double& y, double& z)
{
  assert(sensor && "Processor: line sensor intercept didn't receive a sensor");

  // Normal to the plane
  double n[3] = { 0 };
  sensor->getNormalVector(n[0], n[1], n[2]);

  // A point on the plane
  double p0[3] = { 0 };
  sensor->getGlobalOrigin(p0[0], p0[1], p0[2]);

  // A point on the line
  double l0[3] = { 0 };
  l0[0] = posX;
  l0[1] = posY;
  l0[2] = posZ;

  // The direction of the line
  double l[3] = { 0 };
  l[0] = slopeX;
  l[1] = slopeY;
  l[2] = 1.0;

  // (p0 - l0) dot n
  double numerator = 0;
  for (int i = 0; i < 3; i++)
    numerator += (p0[i] - l0[i]) * n[i];

  // l dot n
  double denominator = 0;
  for (int i = 0; i < 3; i++)
    denominator += l[i] * n[i];

  x = y = z = 0;

  // The distance along the track to the intercept
  double d = numerator / denominator;
   // Check for divison by 0
  if (!(d <= DBL_MAX && d >= -DBL_MAX)) {
    if (VERBOSE) cout << "WARN: line plane intercept divided by zero" << endl;
    return -1;
  }

  // Extrapolate to the intercept
  x = d * l[0] + l0[0];
  y = d * l[1] + l0[1];
  z = d * l[2] + l0[2];

  return 0;
}

int trackSensorIntercept(const Storage::Track* track,
                         const Mechanics::Sensor* sensor,
                         double& x, double& y, double& z)
{
  assert(track && sensor &&
         "Processors: track sensor intercept recieved a null track and/or sensor");

  x = 0, y = 0, z = 0;
  return lineSensorIntercept(track->getOriginX(), track->getOriginY(), 0,
                             track->getSlopeX(), track->getSlopeY(),
                             sensor, x, y, z);
}


void trackClusterDistance(const Storage::Track* track,
                          const Storage::Cluster* cluster,
                          const Mechanics::Sensor* sensor,
                          double& distX, double& distY,
                          double& errX, double& errY)
{
  distX = 0, distY = 0;

  double tx = 0, ty = 0, tz = 0;
  trackSensorIntercept(track, sensor, tx, ty, tz);
  double tex = 0, tey = 0;
  trackError(track, tz, tex, tey);

  errX = sqrt(pow(tex, 2) + pow(cluster->getPosErrX(), 2));
  errY = sqrt(pow(tey, 2) + pow(cluster->getPosErrY(), 2));

  distX = tx - cluster->getPosX();
  distY = ty - cluster->getPosY();
}


void trackError(const Storage::Track* track, double z, double& errX, double& errY)
{
  errX = sqrt(pow(track->getOriginErrX(), 2) +
              pow(z, 2) * pow(track->getSlopeErrX(), 2) +
              2 * z * track->getCovarianceX());
  errY = sqrt(pow(track->getOriginErrY(), 2) +
              pow(z, 2) * pow(track->getSlopeErrY(), 2) +
              2 * z * track->getCovarianceY());
}

}
