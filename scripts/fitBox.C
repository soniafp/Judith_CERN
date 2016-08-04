double boxfun(double *x, double *par)
{
	return par[0]/2.*(TMath::Erf((x[0]-par[1]+par[2]/2.)/(TMath::Sqrt(2.)*par[3]))+
		TMath::Erf((par[1]+par[2]/2.-x[0])/(TMath::Sqrt(2.)*par[3])));
}

int fitBox(TH1F *g1=NULL){
  
  if(!g1) return 0;
  g1->Scale(1.0/500.0);
  
  double mean = 0.0;
  double rms = 25;
  double minf = mean - 50;
  double maxf = mean + 50;
  
  TF1 * boxfunfunc = new TF1("myboxfun", boxfun, minf, maxf, 4);

  boxfunfunc->SetParameter(0, 1); // Höhe, height
  boxfunfunc->SetParameter(1, 15); // Mittelwert, average
  boxfunfunc->SetParameter(2, 20); // Breite der Box, width of box
  boxfunfunc->SetParameter(3, 5); // Breite des Gauß, width of gaussian
  
  g1->Fit("myboxfun", "RN");
  
  mean = boxfunfunc->GetParameter(1);
  double width = boxfunfunc->GetParameter(2);
  double height = boxfunfunc->GetParameter(0);			
  double TDACStepWidthSigma = boxfunfunc->GetParameter(3);
  double sigma = width/3.464101615137754587054;
  boxfunfunc->SetLineStyle(2);
  boxfunfunc->SetLineWidth(2);
  boxfunfunc->Draw("same");

  stringstream ss;
  ss<<"#sigma=" << boxfunfunc->GetParameter(3);
  
  TLatex *a = new TLatex(0.7, 0.72, ss.str().c_str());
  a->SetNDC();
  a->SetTextFont(42);
  a->SetTextSize(0.04);
  a->SetTextAlign(12);
  a->SetTextColor(kBlack);
  a->Draw();

  //g1->GetYaxis()->SetRangeUser(0.0,1.1);
  g1->SetLineColor(1);
  g1->SetMarkerColor(1);
  TLine *line = new TLine(-100.0,1.0,100.0,1.0);
  line->SetLineStyle(2);
  line->SetLineColor(1);
  line->SetLineWidth(2);
  line->Draw();
    
  //TCanvas* can = new TCanvas("can","",50,50,600,600);
  //g1->Draw();
  //can->Update();
  //can->WaitPrimitive();
  
    
}
