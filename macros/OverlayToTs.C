{

  const char* fileNames[] =
    { "desy01/dut01-01-results.root",
      "desy01/dut01-05-results.root",
      "desy01/dut01-07-results.root",
      "desy01/dut01-06-results.root" };

  const char* objectNames[] =
    { "ClusterInfo/DUTDiamondToT" };

  const unsigned int nfiles = sizeof(fileNames) / sizeof(const char*);
  const unsigned int nobjects = sizeof(objectNames) / sizeof(const char*);

  // Populat the objects vector using all object names from all files
  const double legendItemHeight = 0.08;
  const double legendWidth = 0.25;
  const double legendLeft = 0.7;
  const double legendRight = legendLeft + legendWidth;
  const double legendTop = 0.9;
  const double legendBottom = legendTop - nfiles * nobjects * legendItemHeight;
  TLegend* legend = new TLegend(legendLeft, legendTop, legendRight, legendBottom);
  legend->SetFillStyle(0);
  legend->SetBorderSize(0);

  double maxValue = 0;

  TH1D* plot0 = 0;
  for (unsigned int nfile = 0; nfile < nfiles; nfile++)
  {
    TFile* f = new TFile(fileNames[nfile], "READ");
    for (unsigned int nobject = 0; nobject < nobjects; nobject++)
    {
      TH1D* tot = 0;
      TH1D* plot = 0;
      TH1D* norm = 0;
      f->GetObject(objectNames[nobject], tot);
      f->GetObject("ClusterInfo/DUTSiliconToT", norm);
      
      // Convert the ToT to a value in electrons, generates the plot 
      if (nfile == 0)
        plot = JudithUtils::ConvertToT(tot, 1100, 8000, 6);
      if (nfile == 1)
        plot = JudithUtils::ConvertToT(tot, 1500, 8000, 6);
      if (nfile == 2)
        plot = JudithUtils::ConvertToT(tot, 2000, 8000, 6);
      if (nfile == 3)
        plot = JudithUtils::ConvertToT(tot, 2500, 8000, 6);
      
      plot->Scale(1.0 / norm);
      plot->GetXaxis()->SetRangeUser(0,15);
      plot->GetYaxis()->SetTitle("Fraction of clusters");

      Int_t color = 2000 + (nobject + nfile * nobjects) % 5 + 1;
      plot->SetLineColor(color);
      plot->SetFillStyle(0);
      if (!plot0)
      {
        plot0 = plot;
        plot0->Draw();
        maxValue = plot->GetMaximum();
      }
      else 
      {
        plot->Draw("SAME");
        if (plot->GetMaximum() > maxValue)
        {
          maxValue = plot->GetMaximum();
          plot0->GetYaxis()->SetRangeUser(0, maxValue * 1.1);
          gPad->Modified();
        }
      } 

      if (nfile == 0)
        legend->AddEntry(plot, "1100e threshold", "F");
      if (nfile == 1)
        legend->AddEntry(plot, "1500e threshold", "F");
      if (nfile == 2)
        legend->AddEntry(plot, "2000e threshold", "F");
      if (nfile == 3)
        legend->AddEntry(plot, "2500e threshold", "F");
    }
  }

  legend->Draw();
}
