/////////////////////////////////////////////////////////////////////
////															 ////
//// Example 1: Get a threshold map and plot it in 2D histogram  ////
//// Meant to provide examples on very low level how to get data ////
//// from RootDBD data files as used in USBpix.					 ////
//// 															 ////
//// Author: Malte	(with huge support from Jörn ;-), thanks! )  ////
////															 ////
/////////////////////////////////////////////////////////////////////


#include <DataContainer/PixDBData.h>
#include "PixConfDBInterface/PixConfDBInterface.h"
#include "PixConfDBInterface/RootDB.h"
#include "PixController/PixScan.h"
#include "GeneralDBfunctions.h"

#include "AtlasLabels.h"
#include "AtlasStyle.h"

#include <TH2F.h>
#include <TF1.h>
#include <TGraph.h> 
#include <TGraphErrors.h> 
#include <TApplication.h>
#include <TStyle.h>
#include <TPaveText.h>
#include <TPaletteAxis.h>
#include <TColor.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TMath.h>
#include <TLatex.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace PixLib;
using namespace std;

string ScanName, GroupName, ModuleName;

double boxfun(double *x, double *par)
{
	return par[0]/2.*(TMath::Erf((x[0]-par[1]+par[2]/2.)/(TMath::Sqrt(2.)*par[3]))+
		TMath::Erf((par[1]+par[2]/2.-x[0])/(TMath::Sqrt(2.)*par[3])));
}
void debug(string bla){
	cout << "DEBUG: " << bla << endl;
}
void SetAtlasStyle ()
{
  static TStyle* atlasStyle = 0;
  std::cout << "\nApplying ATLAS style settings...\n" << std::endl ;
  if ( atlasStyle==0 ) atlasStyle = AtlasStyle();
  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
}
TStyle* AtlasStyle() 
{
  TStyle *atlasStyle = new TStyle("ATLAS","Atlas style");

  // use plain black on white colors
  Int_t icol=0; // WHITE
  atlasStyle->SetFrameBorderMode(icol);
  atlasStyle->SetFrameFillColor(icol);
  atlasStyle->SetCanvasBorderMode(icol);
  atlasStyle->SetCanvasColor(icol);
  atlasStyle->SetPadBorderMode(icol);
  atlasStyle->SetPadColor(icol);
  atlasStyle->SetStatColor(icol);
  //atlasStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

  // set the paper & margin sizes
  atlasStyle->SetPaperSize(20,26);

  // set margin sizes
  atlasStyle->SetPadTopMargin(0.05);
  atlasStyle->SetPadRightMargin(0.05);
  atlasStyle->SetPadBottomMargin(0.16);
  atlasStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  atlasStyle->SetTitleXOffset(1.4);
  atlasStyle->SetTitleYOffset(1.4);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font=42; // Helvetica
  Double_t tsize=0.05;
  atlasStyle->SetTextFont(font);

  atlasStyle->SetTextSize(tsize);
  atlasStyle->SetLabelFont(font,"x");
  atlasStyle->SetTitleFont(font,"x");
  atlasStyle->SetLabelFont(font,"y");
  atlasStyle->SetTitleFont(font,"y");
  atlasStyle->SetLabelFont(font,"z");
  atlasStyle->SetTitleFont(font,"z");
  
  atlasStyle->SetLabelSize(tsize,"x");
  atlasStyle->SetTitleSize(tsize,"x");
  atlasStyle->SetLabelSize(tsize,"y");
  atlasStyle->SetTitleSize(tsize,"y");
  atlasStyle->SetLabelSize(tsize,"z");
  atlasStyle->SetTitleSize(tsize,"z");

  // use bold lines and markers
  atlasStyle->SetMarkerStyle(20);
  atlasStyle->SetMarkerSize(1.2);
  atlasStyle->SetHistLineWidth(2.);
  atlasStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

  // get rid of X error bars 
  //atlasStyle->SetErrorX(0.001);
  // get rid of error bar caps
  atlasStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  atlasStyle->SetOptTitle(0);
  //atlasStyle->SetOptStat(1111);
  atlasStyle->SetOptStat(0);
  //atlasStyle->SetOptFit(1111);
  atlasStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  atlasStyle->SetPadTickX(1);
  atlasStyle->SetPadTickY(1);

  return atlasStyle;

}
void ATLASLabel(Double_t x,Double_t y,const char* text,Color_t color) 
{
  TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextColor(color);

  double delx = 0.115*696*gPad->GetWh()/(472*gPad->GetWw());

  l.DrawLatex(x,y,"ATLAS");
  if (text) {
    TLatex p; 
    p.SetNDC();
    p.SetTextFont(42);
    p.SetTextColor(color);
    p.DrawLatex(x+delx,y,text);
    //    p.DrawLatex(x,y,"#sqrt{s}=900GeV");
  }
}
void ATLASLabelOld(Double_t x,Double_t y,bool Preliminary,Color_t color) 
{
  TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize); 
  l.SetNDC();
  l.SetTextFont(72);
  l.SetTextColor(color);
  l.DrawLatex(x,y,"ATLAS");
  if (Preliminary) {
    TLatex p; 
    p.SetNDC();
    p.SetTextFont(42);
    p.SetTextColor(color);
    p.DrawLatex(x+0.115,y,"Preliminary");
  }
}
void ATLASVersion(const char* version,Double_t x,Double_t y,Color_t color) 
{

  if (version) {
    char versionString[100];
    sprintf(versionString,"Version %s",version);
    TLatex l;
    l.SetTextAlign(22); 
    l.SetTextSize(0.04); 
    l.SetNDC();
    l.SetTextFont(72);
    l.SetTextColor(color);
    l.DrawLatex(x,y,versionString);
  }
}

void flipHistoCont(TH2F *hi){ // only use for FE-I4 (336 rows), otherwise will produce nonsense
  for(int i=1;i<=hi->GetNbinsX();i++){
    for(int j=1;j<169;j++){ // 168 is the half of 336
      float temp = hi->GetCellContent(i,j);
      int j_temp = 336-j+1;
      hi->SetBinContent(i,j,hi->GetCellContent(i,j_temp));
      hi->SetBinContent(i,j_temp,temp);
    }
  }
  hi->GetYaxis()->SetBinLabel(336,"0");
  hi->GetYaxis()->SetBinLabel(236,"100");
  hi->GetYaxis()->SetBinLabel(136,"200");
  hi->GetYaxis()->SetBinLabel(36,"300");
  hi->GetYaxis()->SetLabelSize(1.25*hi->GetYaxis()->GetLabelSize());
  hi->GetYaxis()->SetTickLength(0.666666*hi->GetYaxis()->GetTickLength());
}

void setSTcontrolPaletteStyle(int pPaletteStyle)
{
	debug("setSTcontrolPaletteStyle");
	float tColorFraction;
	TColor* tColor ;
	int	palette[100];
	switch(pPaletteStyle){
		case 0:	// black-red-yellow
			for(int i=0;i<60;++i){	// start with black-to-red
				tColorFraction = (float) i/60;
				if(!gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,0,0,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,0,0);
				}
				palette[i]=201+i;
			}
			for(int i=0;i<40;++i){	// red-to-yellow now
				tColorFraction = (float)i/40;
				if(!gROOT->GetColor(261+i))
					tColor = new TColor (261+i,1,tColorFraction,0,"");
				else{
					tColor = gROOT->GetColor(261+i);
					tColor->SetRGB(1,tColorFraction,0);
				}
				palette[i+60]=261+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 1:// black-red, *no* yellow
			for(int i=0;i<100;++i){
				tColorFraction = (float)i/100;
				if(!gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,0,0,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,0,0);
				}
				palette[i]=201+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 2:// rainbow
			gStyle->SetPalette(1);
			break;
		case 3:// black-blue-purple-red-orange-yellow-white (TurboDAQ style)
			for(int i=0;i<20;++i){ // black to blue
				tColorFraction = (float)i/20;
				if(! gROOT->GetColor(201+i))
					tColor = new TColor (201+i,0,0,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(0,0,tColorFraction);
				}
				palette[i]=201+i;
			}
			for(int i=0;i<20;++i){	// blue to purple
				tColorFraction = (float) 0.8*(float)i/20;
				if(! gROOT->GetColor(221+i))
					tColor = new TColor (221+i,tColorFraction,0,1,"");
				else{
					tColor = gROOT->GetColor(221+i);
					tColor->SetRGB(tColorFraction,0,1);
			}
			palette[20+i]=221+i;
			}
			for(int i=0;i<20;++i){	// purple to red
				tColorFraction = (float)i/20;
				if(! gROOT->GetColor(241+i))
					tColor = new TColor(241+i,(float) 0.8+(float) 0.2*tColorFraction,0,1-tColorFraction,"");
				else{
					tColor = gROOT->GetColor(241+i);
					tColor->SetRGB((float) 0.8+(float) 0.2*tColorFraction,0,1-tColorFraction);
				}
				palette[40+i]=241+i;
			}
			for(int i=0;i<25;++i){ // red to orange to yellow
				tColorFraction = (float)i/25;
				if(!gROOT->GetColor(261+i))
					tColor = new TColor (261+i,1,tColorFraction,0,"");
				else{
					tColor = gROOT->GetColor(261+i);
					tColor->SetRGB(1,tColorFraction,0);
				}
				palette[60+i]=261+i;
			}
			for(int i=0;i<15;++i){ // yellow to white
				tColorFraction = (float)i/15;
				if(!gROOT->GetColor(286+i))
					tColor = new TColor (286+i,1,1,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(286+i);
					tColor->SetRGB(1,1,tColorFraction);
				}
				palette[80+i]=286+i;
			}
			gStyle->SetPalette(100,palette);
			break;
		case 4:	// black-white
			for(int i=0;i<90;++i){
				tColorFraction = (float)i/100;
				if(! gROOT->GetColor(201+i))
					tColor = new TColor (201+i,tColorFraction,tColorFraction,tColorFraction,"");
				else{
					tColor = gROOT->GetColor(201+i);
					tColor->SetRGB(tColorFraction,tColorFraction,tColorFraction);
				}
				palette[i]=201+i;
			}
			gStyle->SetPalette(90,palette);
			break;
		default:
			gStyle->SetPalette(100);
	}
}

void listScans(string fname, string label){	// this function explains how to loop over DBInquires and get the scan labels. Searching for correct one and plotting it.
string sname, gname, mname;
  RootDB *db = new RootDB(fname.c_str());
  DBInquire *root = db->readRootRecord(1);
  for(recordIterator i = root->recordBegin(); i!=root->recordEnd();i++){
    if((*i)->getName()=="PixScanResult"){
      sname = (*i)->getDecName();
       getDecNameCore(sname);
      cout << "NEW SCAN: \"" << sname << "\"" << endl;
      for(recordIterator ii = (*i)->recordBegin(); ii!=(*i)->recordEnd();ii++){
	if((*ii)->getName()=="PixModuleGroup"){
	  gname = (*ii)->getDecName();
	  getDecNameCore(gname);
	  cout << "  with module group: " <<  gname << endl;
	  for(recordIterator iii = (*ii)->recordBegin(); iii!=(*ii)->recordEnd();iii++){
	    if((*iii)->getName()=="PixModule"){
	      mname = (*iii)->getDecName();
	      getDecNameCore(mname);
	      cout << "     with module: " <<  mname << endl;
	    }
	  }
	}
	}
	}
	 
	if (sname == label)
	{
		ScanName = sname;
		GroupName = gname;
		ModuleName = mname;
		cout << "found scan: " << ScanName << ":" << GroupName << ":" << ModuleName << endl;
		break;
	}
	 else
	 {
		 cout << "wrong scan" << endl << endl;
	 }	
  }
  delete db;
}


void plotAnalogMap(string fname, bool isInt){ // how to get the data and plot it. Still to do: make histogram beatiful ;-)

	double Mean[256];
	double Sigma[256];
	double Steps[256];
	double Xerrors[256];
	for (int step = 0; step < 256; step++)
	{
		Mean[step] = 0;
		Sigma[step] = 0;
		Steps[step] = step / 1.71;
		Xerrors[step] = 0;
	}

	

	TApplication app("app",NULL,NULL);
	SetAtlasStyle();

	cout << "\n\n\t\t\t" << ScanName << GroupName << ModuleName << "\n\n" << endl;
	PixDBData data("scan", (fname+":/"+ScanName+"/"+GroupName).c_str(), ModuleName.c_str());
	TH1D* tTOT = new TH1D("tHist_thr", "", 351, -0.05, 35.05);	//1d hist of threshold to fit the distribution with a gaus

	for (int step = 0; step < 64; step++)
	{
		int step_array[4] = {step,1,0,0};
		PixLib::Histo * myData = data.getGenericPixLibHisto(PixScan::OCCUPANCY,step_array);
		cout << endl << "got data" << endl << endl;	
		if (myData!=0)
		{
			cout << "myData!=0" << endl;
		}
		else
			return;


		tTOT->Fill((*myData)(35, 36));


		Mean[step] = (*myData)(35, 36) / 25.0;
		//cout << "\t" << tTOT->GetMean() << "\t" << tTOT->GetRMS() << endl;
		Sigma[step] = 0;
		//Steps[step] = step;
		Xerrors[step] = 0;
	}

	gStyle->SetOptStat("0");

	//getPlotSetting("STcontrolColorPalette", tPaletteSetting);
	setSTcontrolPaletteStyle(0);

	debug("PlotHists");

	int w = 800;	//canvas width in pixel
	int h = 600;	//canvas height in pixel

	TCanvas* c1 = new TCanvas("c1", "c1", w, h);	//creation of main canvas

	TPaveText* p1 = new TPaveText(c1->GetLeftMargin(),1.-c1->GetTopMargin() + 0.05,1.-c1->GetRightMargin(),0.97,"brNDC");	//text box for the title


	//style set up for the plot title

	p1->SetBorderSize(0);
	p1->SetLineWidth(1);
	p1->SetTextFont(42);
	p1->SetFillColor(0);
	p1->AddText("TOT vs. global feedback current");
	p1->SetTextSize((float)0.09);
	TGraph* g1 = new TGraph(64, Steps, Mean);	//graph with channel on x-axis and threshold on y-axis	


	double mean = 25;
	double rms = 25;
	double minf = mean - 50;
	double maxf = mean + 50;

	TF1 * boxfunfunc = new TF1("myboxfun", boxfun, minf, maxf, 4);

	boxfunfunc->SetParameter(0, 1); // Höhe
	boxfunfunc->SetParameter(1, 15); // Mittelwert
	boxfunfunc->SetParameter(2, 20); // Breite der Box
	boxfunfunc->SetParameter(3, 5); // Breite des Gauß

	g1->Fit("myboxfun", "RN");
	
	mean = boxfunfunc->GetParameter(1);
	double width = boxfunfunc->GetParameter(2);
	double height = boxfunfunc->GetParameter(0);			
	double TDACStepWidthSigma = boxfunfunc->GetParameter(3);
	double sigma = width/3.464101615137754587054;

//	c1->Divide(1,2);	//division into 3 sub plots

	//c1->SetLeftMargin((float) 0.15);
	//c1->SetRightMargin((float) 0.5);
	//c1->SetTopMargin((float) 0.5);
	//c1->SetBottomMargin((float) 0.5);
	//c1->SetBorderMode(0);

	c1->cd(1);	//enter first sub pad



	g1->GetYaxis()->SetTitle("Hit detection probability");

	g1->GetXaxis()->SetTitle("Injection delay [ns]");


    g1->GetXaxis()->SetLabelFont(42);
    g1->GetXaxis()->SetLabelSize(0.05);
    g1->GetXaxis()->SetTitleSize(0.05);
    g1->GetXaxis()->SetTitleOffset(1.4);
    g1->GetXaxis()->SetTitleFont(42);
    g1->GetYaxis()->SetLabelFont(42);
    g1->GetYaxis()->SetLabelSize(0.05);
    g1->GetYaxis()->SetTitleSize(0.05);
    g1->GetYaxis()->SetTitleOffset(1.4);
    g1->GetYaxis()->SetTitleFont(42);



	//g1->SetMarkerColor(1);
	g1->GetXaxis()->SetRangeUser(0, 38);
	g1->GetYaxis()->SetRangeUser(0, 1.25);

	g1->SetLineWidth(1.5);
	g1->SetFillStyle(3006);
	g1->SetTitle("");
	g1->SetMarkerStyle(20);
	g1->SetMarkerSize(1);
	g1->SetLineWidth(1);
	g1->SetMarkerColor(kGreen+3);
	g1->SetLineColor(kGreen+3);
	g1->Draw("AP");

	boxfunfunc->SetLineColor(kRed);
	boxfunfunc->SetLineWidth(2.5);
	boxfunfunc->Draw("SAME");

	TGraph * tMaxLimit = new TGraph(2);
	tMaxLimit->SetPoint(0, mean - (width/2.0), 0);
	tMaxLimit->SetPoint(1, mean - (width/2.0), 1.05);
	tMaxLimit->SetLineStyle(5);
	tMaxLimit->SetLineColor(kBlack);
	tMaxLimit->SetLineWidth(2);
	tMaxLimit->Draw("SAMEL");

	TGraph * tMaxLimit1 = new TGraph(2);
	tMaxLimit1->SetPoint(0, mean - (width/2.0) + 5, 0);
	tMaxLimit1->SetPoint(1, mean - (width/2.0) + 5, 1.05);
	tMaxLimit1->SetLineStyle(2);
	tMaxLimit1->SetLineColor(kBlue);
	tMaxLimit1->SetLineWidth(2);
	tMaxLimit1->Draw("SAMEL");

	//TGraph *g2 = new TGraph(256, Steps, Mean);
	//g2->SetLineWidth(3);
	//g2->SetLineColor(kBlue);
	//g2->SetLineStyle(1);
	//g2->Draw("P");

	//TLegend *l = new TLegend(0.74,0.45,0.98,0.85);		//legend
	TLegend *l = new TLegend(0.43,0.35,0.67,0.67,NULL,"trNDC");		//legend
	l->SetFillColor(0);
	l->SetLineColor(0);
	l->SetTextFont(42);
	l->SetTextSize((float)0.05);
	l->SetBorderSize(1);
	l->SetLineWidth(1);

	//l->AddEntry(tHist_thr, Form("histogram, entries: %4.0f", tHist_thr->GetEntries()), "f");
	l->AddEntry(boxfunfunc, "Box function fit", "l");
	l->AddEntry(boxfunfunc, Form("mean:   %4.2f ns"/* +- %1.2f"*/, mean/*, sigma*/), "");
	l->AddEntry(boxfunfunc, Form("#sigma:            %4.2f ns"/* +- %1.2f"*/, sigma/*, sigma*/), "");
	l->AddEntry(boxfunfunc, Form("width:    %4.2f ns"/* +- %1.2f"*/, width/*, sigma*/), "");
	l->AddEntry(tMaxLimit, "turn-on time", "l");
	l->AddEntry(tMaxLimit1, "t0 time", "l");
	l->Draw();


	if(isInt){
	ATLASLabel(0.2,0.875,"Internal");
	gPad->RedrawAxis();
	c1->SaveAs("T0_Scan_int.pdf");
	c1->SaveAs("T0_Scan_int.eps");
	c1->SaveAs("T0_Scan_int.root");
	c1->SaveAs("T0_Scan_int.C");
	}else{
	ATLASLabel(0.2,0.875,"Preliminary");
	gPad->RedrawAxis();
	c1->SaveAs("T0_Scan.pdf");
	c1->SaveAs("T0_Scan.eps");
	c1->SaveAs("T0_Scan.root");
	c1->SaveAs("T0_Scan.C");
	}
	//p1->Draw();	//draw title into the same canvas

	gROOT->SetBatch(false);

	//cout << "Press ctrl-C to stop" << endl;
	//app.Run();
}

void main(){
		listScans("./Timing.root", "T0_SCAN");
		plotAnalogMap("./Timing.root", true);
		//plotAnalogMap("./Timing.root", true);
}

