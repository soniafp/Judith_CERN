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

void testbEffVsVoltageATLAS(bool config = true)

{
  
    
    if(config) setStyle();
//#ifdef __CINT__
//  gROOT->LoadMacro("AtlasUtils.C");
//#endif
    
  //SetAtlasStyle();

    // Corrections if needed
    //float scale = 1.0/(1.0-0.03094871794871795);
    float scale = 1.0; // new numbers already have the 1./0.97 applied
    bool doCorrection=false;
    double correction_20 = 1.0/0.9882; ///0.995/0.995;
    double correction_80 = 1.0/0.8675; // /0.94/0.94;  
    if(!doCorrection){
      correction_20=1.0;
      correction_80=1.0;
    }
    TCanvas* c1 = new TCanvas("c1","testbeam efficiency",50,50,600,600);
    TMultiGraph *mg = new TMultiGraph();

  
  
  ///Graph 1 -- 25% pixel size
  const Int_t n1 = 3;
  Double_t x1[n1]  = {60, 90, 120};
  //Double_t y1[n1]  = { 0.439011,0.812868, 0.895039};
  //Double_t y1[n1]  = { 0.456,0.853522, 0.942};
  //Double_t y1[n1]  = { 0.660*scale,0.863522*scale, 0.942*scale}; // old 36%
  //Double_t y1[n1]  = { 0.643*scale,0.913*0.97*scale, 0.942*scale}; // 36%
  Double_t y1[n1]  = { 0.760*scale*correction_20,0.969*scale*correction_20, 0.979*scale*correction_20}; // 20%- t0 cut from 500-700
  //Double_t y1[n1]  = { 0.732*scale,0.925*0.97*scale, 0.944*scale}; // 20%
  //Double_t ey1[n1] = {0.008, 0.007, 0.004};
  Double_t eyh1[n1] = {0.017*scale, 0.007*scale, 0.007*scale}; // variance systematic is the RMS on the mean efficiency within +/-4um
  Double_t eyl1[n1] = {0.017*scale, 0.007*scale, 0.007*scale}; // variance systematic is the RMS on the mean efficiency within +/-4um  

  if(correction_20>1.0){
    for(unsigned k=0; k<n1; ++k){
      eyh1[k]=sqrt(eyh1[k]*eyh1[k]+0.006*0.006); // adding the uncertainties on the corrections.
      eyl1[k]=sqrt(eyl1[k]*eyl1[k]+0.0105*0.0105); // adding the uncertainties on the corrections.      
    }
  }
  
  //Double_t ey1[n1] = {0.008, 0.007, 0.004}; //20%
  Double_t ex1[n1] = {0.0, 0.0, 0.0};  
  TGraphAsymmErrors *gr1 = new TGraphAsymmErrors(n1,x1,y1,ex1,ex1,eyl1,eyh1);
  gr1->SetName("gr1");
  gr1->SetTitle("Hit detection efficiency vs Voltage");
    gr1->GetXaxis()->SetTitle("Bias Voltage [V]");
    gr1->GetYaxis()->SetTitle("Hit Efficiency [%]");
    gr1->GetYaxis()->SetRangeUser(0.4,1.1);
    if(correction_20>1.0){
    gr1->GetYaxis()->SetRangeUser(0.4,1.1);
    }
  gr1->SetMarkerColor(1);
    gr1->SetLineColor(1);
    gr1->SetFillColor(0);
    
    //gr1->SetMarkerStyle(20);
    
  gr1->SetLineWidth(2);
  gr1->SetMarkerStyle(22);
  gr1->Draw();
  mg->Add(gr1);
    
  ////////Graph 2  -- 80% pixel size
  const Int_t n2 = 3;
  Double_t x2[n2]  = {60, 90, 120};
  //Double_t y2[n2]  = { 0.387501,0.720055, 0.790867};
  //Double_t y2[n2]  = {0.39 , 0.731, 0.815}; // off by 6um
  //Double_t y2[n2]  = {0.488*0.97*scale , 0.811*0.97*scale, 0.835*scale};
  Double_t y2[n2]  = {0.499*scale*correction_80 , 0.842*scale*correction_80, 0.864*scale*correction_80};  
  //Double_t y2[n2]  = {0.407*scale , 0.739*scale, 0.815*scale};      
  Double_t ex2[n2] = {0.0, 0.0, 0.0};
  //Double_t ey2[n2] = {0.004, 0.02, 0.001};
  Double_t eyl2[n1] = {0.015*scale, 0.023*scale, 0.016*scale}; // variance systematic is the RMS on the mean efficiency within +/-4um
  Double_t eyh2[n1] = {0.015*scale, 0.023*scale, 0.016*scale}; // variance systematic is the RMS on the mean efficiency within +/-4um  
  if(correction_80>1.0){
    for(unsigned k=0; k<n2; ++k){
      eyh2[k]=sqrt(eyh2[k]*eyh2[k]+0.0238*0.0238);// adding the uncertainties on the corrections.
      eyl2[k]=sqrt(eyl2[k]*eyl2[k]+0.0245*0.0245);// adding the uncertainties on the corrections.      
    }
  }

  
  TGraphAsymmErrors *gr2 = new TGraphAsymmErrors(n2,x2,y2,ex2,ex2,eyl2,eyh2);
  gr2->SetName("gr2");
  gr2->SetMarkerColor(2);
    gr2->SetLineColor(2);
    gr2->SetFillColor(0);
    
  gr2->SetLineWidth(2);
  gr2->SetMarkerStyle(21);
    gr2->Draw("same pl");
    gr1->Draw("same pl");
  mg->Add(gr2);
    
    TLegend *leg = new TLegend(0.45, 0.3, 0.85, 0.5);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    //leg->AddEntry(gr1, "36% Pixel Size");
    leg->AddEntry(gr1, "20% Pixel Size");
    leg->AddEntry(gr2, "80% Pixel Size");
    leg->Draw();

    if(correction_20>1.0||true){
      TLine *lin = new TLine(54.0,1.0,126.0,1.0);
      lin->SetLineStyle(2);
      lin->SetLineWidth(2);
      lin->Draw();
    }
    
    //mg->Draw();
    c1->Update();
    c1->WaitPrimitive();

    if(!doCorrection) c1->SaveAs("notcorrected.pdf");
    else c1->SaveAs("corrected.pdf");
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
