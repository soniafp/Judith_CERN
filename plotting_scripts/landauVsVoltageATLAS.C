#include <iostream>
#include <cmath>

#include "Rtypes.h"

//#include "AtlasUtils.h"
//#include "AtlasStyle.h"
//#include "AtlasLabels.h"
// #ifndef __CINT__
// these are not headers - do not treat them as such
// #include "AtlasStyle.C"
// #include "AtlasLabels.C"
// #endif

#include "TCanvas.h"
#include "TFile.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TGraphErrors.h"

using namespace std;

const Int_t GMAX=864;

const int nren=3;
static const double mur[nren] = {1.0,0.25,4.0};
static const double muf[nren] = {1.0,0.25,4.0};
const unsigned int NUMPDF=41;

TGraphErrors* GetGraph(Int_t ir, Int_t ifs,Int_t icut, Int_t ipdf);
TMultiGraph *mg;
TGraphErrors *gr1;
TGraphErrors *gr2;

void setStyle(){
    
    gROOT->LoadMacro("/Users/schae/testarea/CAFAna/HWWMVACode/atlasstyle-00-03-05/AtlasStyle.C");
    gROOT->LoadMacro("/Users/schae/testarea/CAFAna/HWWMVACode/atlasstyle-00-03-05/AtlasUtils.C");
    SetAtlasStyle();
}

void landauVsVoltageATLAS(bool config = true)

{
  
    
    if(config) setStyle();
//#ifdef __CINT__
//  gROOT->LoadMacro("AtlasUtils.C");
//#endif
    
  //SetAtlasStyle();
  
    float scale = 1.0/(1.0-0.03094871794871795);
  
  TCanvas* c1 = new TCanvas("c1","testbeam efficiency",50,50,600,600);
  TMultiGraph *mg = new TMultiGraph();

  
  //gStyle->SetTitleOffset(1.0,"xz");
  gStyle->SetTitleOffset(1.6,"y");
  //gStyle->SetPadLeftMargin(0.26);
  gStyle->SetLabelSize(0.045, "y");
  ///Graph 1 -- 25% pixel size
  const Int_t n1 = 3;
  Double_t x1[n1]  = {60, 90, 120};
  Double_t y1[n1]  = { 0.00344,0.00437, 0.00525}; // peak most probable
  //Double_t y1[n1]  = { 0.00344,0.00454, 0.00551}; // mean from fit
  Double_t ey1[n1] = {0.000503, 0.000820, 0.00103};
  Double_t ex1[n1] = {0.00334, 0.0, 0.0};  
  TGraphErrors *gr1 = new TGraphErrors(n1,x1,y1,ex1,ey1);
  gr1->SetName("gr1");
  gr1->SetTitle("Hit detection efficiency vs Voltage");
  gr1->GetXaxis()->SetTitle("#sqrt{Bias Voltage [V]}");
  gr1->GetYaxis()->SetTitle("Most Probable Signal [e^{-}]");
  gr1->GetYaxis()->SetRangeUser(0,0.0075);
  gr1->SetMarkerColor(1);
  gr1->SetLineColor(1);
  gr1->SetFillColor(0);
  
  //gr1->SetMarkerStyle(20);
    
  gr1->SetLineWidth(2);
  gr1->SetMarkerStyle(22);
  gr1->Draw();
  mg->Add(gr1);

    
  ////////Graph 2  -- 80% pixel size
  /*
  const Int_t n2 = 3;
  Double_t x2[n2]  = {60, 90, 120};
  //Double_t y2[n2]  = { 0.387501,0.720055, 0.790867};
  //Double_t y2[n2]  = {0.39 , 0.731, 0.815}; // off by 6um
  Double_t y2[n2]  = {0.488*0.97*scale , 0.811*0.97*scale, 0.835*scale};
  //Double_t y2[n2]  = {0.407*scale , 0.739*scale, 0.815*scale};      
  Double_t ex2[n2] = {0.0, 0.0, 0.0};
  Double_t ey2[n2] = {0.004, 0.02, 0.001};  
  TGraphErrors *gr2 = new TGraphErrors(n2,x2,y2,ex2,ey2);
  gr2->SetName("gr2");
  gr2->SetMarkerColor(2);
    gr2->SetLineColor(2);
    gr2->SetFillColor(0);
    
  gr2->SetLineWidth(2);
  gr2->SetMarkerStyle(21);
    gr2->Draw("same pl");
  mg->Add(gr2);
    */
    //TLegend *leg = new TLegend(0.45, 0.3, 0.85, 0.5);
    //leg->SetBorderSize(0);
    //leg->SetFillStyle(0);
    //leg->AddEntry(gr1, "36% Pixel Size");
    //leg->AddEntry(gr2, "80% Pixel Size");
    //leg->Draw();
    
    //mg->Draw();
    c1->Update();
    c1->WaitPrimitive();
    
  
    //myText(       0.3,  0.85, 1, "#sqrt{s}= 14 TeV");
  //myText(       0.57, 0.85, 1, "|#eta_{jet}|<0.5");
  //myMarkerText( 0.55, 0.75, 1, 20, "Data 2009",1.3);

}


#ifndef __CINT__
int main() {
  testbEffVsVoltageATLAS();
  //gPad->Print("basic.png");
  return 0;
}
#endif
