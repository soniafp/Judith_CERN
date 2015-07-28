// Temporary macro

{  
  const double x1 = -6000;
  const double x2 = 6000;
  const double y1 = -3000;
  const double y2 = 3000;  
  
  int step = 0;
  const int nsteps = 6;
  
  double x[nsteps];
  double y[nsteps];
  double eyl[nsteps];
  double eyh[nsteps];
  
  double xc[nsteps];
  double yc[nsteps];
  double eylc[nsteps];
  double eyhc[nsteps];
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 7, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist7");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 4;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }

  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 8, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist8");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 5;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 9, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist9");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 6;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 10, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist10");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 7;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 11, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist11");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 8;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 12, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist12");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 9;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  {
    double corr, corrErr;
    JudithUtils::CalculateBackground(corr, corrErr, 20, "../sys-max-dist.root", "Matching/DUTDiamondDistance");
  
    TEfficiency* eff = JudithUtils::GetEfficiency("../sys-max-dist.root", "Efficiency/DUTDiamondMapMaxDist20");
  
    double e, u, l;
    JudithUtils::CalculateEfficiency(eff, e, u, l, 0, 0,
                                     x1, x2, y1, y2);  
  
    double ec, uc, lc;
    JudithUtils::CalculateEfficiency(eff, ec, uc, lc, corr, corrErr,
                                     x1, x2, y1, y2);
    
    x[step] = 10;
    y[step] = e;
    eyl[step] = l;
    eyh[step] = u;
        
    xc[step] = x[step];
    yc[step] = ec;
    eylc[step] = lc;
    eyhc[step] = uc;
    
    step++;
  }
  
  TCanvas* can = new TCanvas();
  can->SetLeftMargin(0.2);
  
  gStyle->SetEndErrorSize(4);
  TGaxis::SetMaxDigits(3);
  
  TMultiGraph* mg = new TMultiGraph();
  
  TGraphAsymmErrors* graph1 = new TGraphAsymmErrors(
    nsteps, x, y, 0, 0, eyl, eyh);
  
  graph1->SetMarkerColor(2001);
  graph1->SetLineColor(2001);
  graph1->SetLineWidth(2);
  
  TGraphAsymmErrors* graph2 = new TGraphAsymmErrors(
    nsteps, xc, yc, 0, 0, eylc, eyhc);
   
  graph2->SetMarkerColor(2005);
  graph2->SetLineColor(2005);
  graph2->SetLineWidth(2);
  
  mg->Add(graph1);
  mg->Add(graph2);
  mg->Draw("AP");
  
  TLegend leg(0.7, 0.80, 0.95, 0.93);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  leg->AddEntry(graph1, "Uncorrected", "LPE");
  leg->AddEntry(graph2, "Corrected", "LPE");
  leg->Draw();

  mg->GetXaxis()->SetTitle("Track cluster distance [standard deviation]");
  mg->GetYaxis()->SetTitle("Efficiency");
  
  mg->GetYaxis()->SetTitleOffset(1.8);
  
  gPad->Modified();
  can->Update();
}
