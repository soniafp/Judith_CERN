#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <float.h>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TLatex.h>
#include <TCanvas.h>
#include <TEfficiency.h>
#include <TMath.h>
#include <TDirectory.h>
#include <TClass.h>

using namespace std;

namespace JudithUtils {

TH1D* ConvertToT(const TH1D* tot, double threshold, double tune, unsigned int tuneBin)
{
  const Int_t nbins = tot->GetXaxis()->GetNbins();
  const double scale = (tune - threshold) / (double)tuneBin;
  const double max = (nbins - 1) * scale + threshold + scale / 2.0;
  const double min = threshold - scale / 2.0;
  
  TH1D* converted = new TH1D("ScaledTot",
                             "Deposited Charge Distribution;"
                             "Uncalibrated cluster charge [e];"
                             "Clusters",
                             nbins, min, max);
  
  for (Int_t bin = 1; bin <= nbins; bin++)
  {
   converted->SetBinContent(bin, tot->GetBinContent(bin)); 
  }
  
  return converted;
}

TH1D* RadialToFlat(TH1D* radial)
{
  TH1D* flat = new TH1D(*radial);
  flat->Reset();

  for (Int_t bin = 1; bin <= flat->GetNbinsX(); bin++)
  {
    const double factor = 1.0 / (TMath::Pi() * flat->GetXaxis()->GetBinCenter(bin));
    const double value = radial->GetBinContent(bin) * factor;
    flat->SetBinContent(bin, value);
  }

  return flat;
}

void DrawMatchingFit(const char* fileName, const char* histName, const char* fitName)
{
  TFile* file = new TFile(fileName, "READ");
  TF1* fit = 0;
  TH1D* hist = 0;
  file->GetObject(histName, hist);
  file->GetObject(fitName, fit);
  if (!fit)
    cout << "WARNING: fit object " << fitName
         << " not found in file " << fileName;
  if (!fit)
    cout << "WARNING: histogram object " << histName
         << " not found in file " << fileName;

  hist->Draw();
  
  fit->SetLineColor(46);
  fit->Draw("SAME");

  const double textSize = 0.05;
  const double posX = 0.65;
  const double posY = 0.9;

  TLatex l;
  l.SetNDC();
  l.SetTextColor(1);
  l.SetTextSize(textSize);

  double width = fit->GetParameter(0);
  //double widthErr = fit->GetParError(0); 
  double sigma = fit->GetParameter(1);
  //double sigmaErr = fit->GetParError(1); 
  double background = fit->GetParameter(2);
  //double backgroundErr = fit->GetParError(2); 
  double offset = fit->GetParameter(3);
  //double offsetErr = fit->GetParError(3); 

  stringstream ss;
  ss << setprecision(3);
  int i = -1;

  ss.str(""); i++;
  ss << "\\mathrm{width} = " << width;
  l.DrawLatex(posX, posY - i * textSize, ss.str().c_str());

  ss.str(""); i++;
  ss << "\\mathrm{resolution} = " << sigma;
  l.DrawLatex(posX, posY - i * textSize, ss.str().c_str());

  ss.str(""); i++;
  ss << "\\mathrm{background} = " << background;
  l.DrawLatex(posX, posY - i * textSize, ss.str().c_str());

  ss.str(""); i++;
  ss << "\\mathrm{offset} = " << offset;
  l.DrawLatex(posX, posY - i * textSize, ss.str().c_str());
}

TEfficiency* GetEfficiency(const char* fileName, const char* objectName)
{
  TFile* file = new TFile(fileName, "READ");
  TEfficiency* eff = 0;
  file->GetObject(objectName, eff);
  if (!eff)
    cout << "WARNING: efficiency object " << objectName
         << " not found in file " << fileName;
  else
    eff->SetDirectory(0);
  delete file;
  return eff;
}

TH2D* GetEfficiencyMap(TEfficiency* efficiency, double maxErr = -1)
{
  if (!efficiency)
  {
    cout << "Warning: null pointer passed to get efficiency map" << endl;
    return 0;
  }

  TH2D* total = (TH2D*)efficiency->GetTotalHistogram();

  TH2D* map = new TH2D(*total);
  map->SetDirectory(0);
  map->Reset();
  
  for (Int_t binx = 1; binx <= total->GetNbinsX(); binx++)
  {
    for (Int_t biny = 1; biny <= total->GetNbinsY(); biny++)
    {
      Int_t bin = total->GetBin(binx, biny);
      if (total->GetBinContent(bin) < 1)
        continue;
      
      const double errUp = efficiency->GetEfficiencyErrorLow(bin);
      const double errLow = efficiency->GetEfficiencyErrorUp(bin);
  
      const double largeErr = errUp > errLow ? errUp : errLow;
      
      if (maxErr > 0 && largeErr > maxErr) continue; 

      double eff = efficiency->GetEfficiency(bin);

      if (eff == 0.0) eff = DBL_MIN;
      map->SetBinContent(bin, eff);
    }
  }

  map->GetZaxis()->SetRangeUser(0,1);
  return map;
}

TH2D* GetEfficiencyErrMap(TEfficiency* efficiency)
{
  if (!efficiency)
  {
    cout << "Warning: null pointer passed to get efficiency map" << endl;
    return 0;
  }

  TH2D* total = (TH2D*)efficiency->GetTotalHistogram();

  TH2D* map = new TH2D(*total);
  map->SetDirectory(0);
  map->Reset();
  map->GetZaxis()->SetTitle("Efficiency uncertainty");
  
  for (Int_t binx = 1; binx <= total->GetNbinsX(); binx++)
  {
    for (Int_t biny = 1; biny <= total->GetNbinsY(); biny++)
    {
      Int_t bin = total->GetBin(binx, biny);
      if (total->GetBinContent(bin) < 1)
        continue;
      
      const double err =
        efficiency->GetEfficiencyErrorLow(bin) / 2.0 +
        efficiency->GetEfficiencyErrorUp(bin) / 2.0;
      map->SetBinContent(bin, err);
    }
  }

  return map;
}


void CalculateBackground(double& correction,
                         double& uncertainty,
                         double cutoff,
                         const char* fileName,
                         const char* objectName)
{
  correction = 0;
  uncertainty = 0;

  cout << "NOTE: the efficiency and matching analyzers should use the same "
          "cuts for this correction to work" << endl;

  TFile* file = new TFile(fileName, "READ");
  TH1D* matchDistance = 0;
  file->GetObject(objectName, matchDistance);
  
  if (!matchDistance)
    cout << "WARNING: matching plot " << objectName
         << " not found in file " << fileName;
  else
    matchDistance->SetDirectory(0);
    
  delete file;
  
  const Int_t cutBin = matchDistance->FindFixBin(cutoff); // Also number of bins in cut
  const Int_t bgBins = matchDistance->GetNbinsX() - cutBin;
  
  // Check that the cut is in the plot and not on the far right edge
  if (bgBins < 1)
  {
    cout << "WARNING: cut off not inside plot range" << endl;
    return;
  }
  
  TH1D* flatDist = RadialToFlat(matchDistance);
  
  // Estimate the background  
  double bgPerBin = 0;
  for (Int_t bin = cutBin + 1; bin <= flatDist->GetNbinsX(); bin++)
    bgPerBin += flatDist->GetBinContent(bin);
  
  bgPerBin /= (double)(bgBins); // Average background per bin
  
  // Estimate the background uncertainty
  double bgErr = 0;
  
  if (bgBins > 1)
  {
    for (Int_t bin = cutBin + 1; bin <= flatDist->GetNbinsX(); bin++)
      bgErr += pow(flatDist->GetBinContent(bin) - bgPerBin, 2);
    
    bgErr = sqrt(1.0 / (bgBins - 1) * bgErr); // RMS
    bgErr /= (double)sqrt(bgBins); // Uncertainty
  }
  else
  {
    bgErr = sqrt(bgPerBin);
  }  
  
  // Correct for the incomplete last bin
  const double binFraction = (cutoff - flatDist->GetBinLowEdge(cutBin)) /
                             flatDist->GetBinWidth(cutBin);

  // Get the number of matches in the cut
  double numMatches = flatDist->Integral(1, cutBin);
  numMatches -= flatDist->GetBinContent(cutBin) * (1 - binFraction);

  // Get the amount of background in the cut
  correction = bgPerBin * (cutBin - 1 + binFraction) / numMatches;
  uncertainty = bgErr * (cutBin - 1 + binFraction) / numMatches;
}

TH2D* CalculateEfficiency(TEfficiency* efficiency,
                          double& eff,
                          double& sigUp,
                          double& sigLow,
                          double correction = 0, double correctionErr = 0,
                          double cutX1 = 0, double cutX2 = 0,
                          double cutY1 = 0, double cutY2 = 0)
{
  eff    = 0;
  sigUp  = 0;
  sigLow = 0;

  if (!efficiency)
  {
    cout << "Warning: null pointer passed to get efficiency map" << endl;
    return 0;
  }

  TH2* total  = (TH2*)efficiency->GetTotalHistogram();
  TH2* passed = (TH2*)efficiency->GetPassedHistogram();

  // If there are cuts, find the bins at which to cut
  Int_t binx1 = 1;
  Int_t binx2 = total->GetNbinsX();

  if (cutX1 != cutX2)
  {
    const Int_t cutBinX1 = total->GetXaxis()->FindFixBin(cutX1);
    const Int_t cutBinX2 = total->GetXaxis()->FindFixBin(cutX2);
    if (cutBinX1 >= binx1 && cutBinX1 <= binx2) binx1 = cutBinX1;
    if (cutBinX2 >= binx1 && cutBinX2 <= binx2) binx2 = cutBinX2;
  }
  
  Int_t biny1 = 1;
  Int_t biny2 = total->GetNbinsY();

  if (cutY1 != cutY2)
  {
    const Int_t cutBinY1 = total->GetYaxis()->FindFixBin(cutY1);
    const Int_t cutBinY2 = total->GetYaxis()->FindFixBin(cutY2);
    if (cutBinY1 >= biny1 && cutBinY1 <= biny2) biny1 = cutBinY1;
    if (cutBinY2 >= biny1 && cutBinY2 <= biny2) biny2 = cutBinY2;
  }

  TEfficiency* overall = new TEfficiency("OverallEfficiency",
                                         "Overall Efficiency",
                                         1, -0.5, 0.5);
  overall->SetDirectory(0);

  unsigned int numTotal = 0;
  unsigned int numPassed = 0;
  unsigned int numTotalCut = 0;
  unsigned int numPassedCut = 0;

  for (Int_t binx = 0; binx <= total->GetNbinsX(); binx++)
  {
    for (Int_t biny = 0; biny <= total->GetNbinsY(); biny++)
    {
      numTotal += total->GetBinContent(binx, biny);
      numPassed += passed->GetBinContent(binx, biny);
      
      if (binx >= binx1 && binx <= binx2 &&
          biny >= biny1 && biny <= biny2)
      {
        numTotalCut += total->GetBinContent(binx, biny);
        numPassedCut += passed->GetBinContent(binx, biny);
      }
    }
  }

  // Remove the fraction of background from the passing tracks
  numPassedCut *= (1 - correction);
  
  // Propagate the correction uncertainty to an efficiency
  correctionErr = correctionErr * numPassedCut / (double)numTotalCut;
  
  for (unsigned int i = 0; i < numTotalCut; i++)
  {
    if (i < numPassedCut) overall->Fill(true, 0);
    else overall->Fill(false, 0);
  }

  Int_t bin = overall->GetTotalHistogram()->FindFixBin(0);
  eff    = overall->GetEfficiency(bin);
  sigUp  = sqrt(pow(overall->GetEfficiencyErrorUp(bin), 2) +
                pow(correctionErr, 2));
  sigLow = sqrt(pow(overall->GetEfficiencyErrorLow(bin), 2) +
                pow(correctionErr, 2));
                
  if (eff - sigLow < 0) sigLow = eff;
  if (eff + sigUp > 1) sigUp = 1 - eff;

  delete overall;

  TH2D* map = GetEfficiencyMap(efficiency);
  map->GetXaxis()->SetRange(binx1, binx2);
  map->GetYaxis()->SetRange(biny1, biny2);

  return map;
}

TH1D* GenerateEfficiencyDistribution(TEfficiency* efficiency,
                                     bool smear = false,
                                     double maxErr = 0)
{
  if (!efficiency)
  {
    cout << "Warning: null pointer passed to generate distribution" << endl;
    return 0;
  }

  TH1D* distribution = new TH1D("EfficiencyDistribution",
                                "Pixel Group Efficiency Distribution"
                                ";Pixel group efficiency"
                                ";Pixel groups / 0.05",
                                20, 0, 1.01);
  distribution->SetDirectory(0);

  // Get efficiency per pixel
  const TH1* values = efficiency->GetTotalHistogram();

  // Loop over all pixel groups
  for (Int_t binx = 1; binx <= values->GetNbinsX(); binx++)
  {
    for (Int_t biny = 1; biny <= values->GetNbinsY(); biny++)
    {
      const Int_t bin = values->GetBin(binx, biny);
      if (values->GetBinContent(binx, biny) < 1) continue;
      const double value = efficiency->GetEfficiency(bin);
      const double sigmaLow = efficiency->GetEfficiencyErrorLow(bin);
      const double sigmaHigh = efficiency->GetEfficiencyErrorUp(bin);

      if (smear)
      {
        // Find the probability of this pixel group being found in all bins of the distribution
        double normalization = 0;
        for (Int_t distBin = 1; distBin <= distribution->GetNbinsX(); distBin++)
        {
          const double evaluate = distribution->GetBinCenter(distBin);
          const double sigma = (evaluate < value) ? sigmaLow : sigmaHigh;
          const double weight = TMath::Gaus(evaluate, value, sigma);
          normalization += weight;
        }
        for (Int_t distBin = 1; distBin <= distribution->GetNbinsX(); distBin++)
        {
          const double evaluate = distribution->GetBinCenter(distBin);
          const double sigma = (evaluate < value) ? sigmaLow : sigmaHigh;
          const double weight = TMath::Gaus(evaluate, value, sigma);
          distribution->Fill(evaluate, weight / normalization);
        }
      }
      else if (maxErr && sigmaLow <= maxErr && sigmaHigh <= maxErr)
      {
        distribution->Fill(value);
      }
    }
  }

  return distribution;
}

}
