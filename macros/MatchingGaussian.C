{
  TFile* file = new TFile("../results.root");
  TH1D* dist = 0;
  file->GetObject("Matching/DUTDiamondDistanceChi10", dist);
  TH1D* flat = JudithUtils::RadialToFlat(dist);
  flat->Scale(1.0/flat->Integral());
  flat->GetYaxis()->SetTitle("Fraction of tracks");

  const double max = dist->GetXaxis()->GetBinUpEdge(dist->GetNbinsX());
  const double min = dist->GetXaxis()->GetBinLowEdge(1);

  // Gaussian centered at 0 with default width of 1 sigma
  TF1* gaus = new TF1("g1", "[0] / ([1] * 2.50662827463) * exp(-0.5 * (x/[1])**2) + [2]");
  gaus->SetParNames("Constant", "Sigma", "Offset");
  gaus->SetParameters(1, 1, 0);
  gaus->SetRange(min, max);

  flat->Fit("g1", "QR0");
 
  TCanvas* can = new TCanvas("can", "can", 800, 600); 
  gaus->SetLineColor(46);
  flat->Draw();
  gaus->Draw("SAME");

  cout << "Sigma : " << gaus->GetParameter("Sigma") << endl;
  cout << "Offset: " << gaus->GetParameter("Offset") << endl;
}
