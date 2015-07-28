{
  const char* fileNames[] =
    { "MDBM01-1500e.root",
      "MDBM02-1100e.root",
      "MDBM03-1500e.root" };

  const char* objectNames[] =
    { "DUTDiamondMap" };

  const unsigned int nfiles = sizeof(fileNames) / sizeof(const char*);
  const unsigned int nobjects = sizeof(objectNames) / sizeof(const char*);

  // Populat the objects vector using all object names from all files
  const double legendItemHeight = 0.08;
  const double legendWidth = 0.25;
  const double legendLeft = 0.2;
  const double legendRight = legendLeft + legendWidth;
  const double legendTop = 0.9;
  const double legendBottom = legendTop - nfiles * nobjects * legendItemHeight;
  TLegend* legend = new TLegend(legendLeft, legendTop, legendRight, legendBottom);
  legend->SetFillStyle(0);
  legend->SetBorderSize(0);

  double maxValue = 0;
  TH1D* plot0;
  for (unsigned int nfile = 0; nfile < nfiles; nfile++)
  {
    for (unsigned int nobject = 0; nobject < nobjects; nobject++)
    {
      TEfficiency* eff =
        JudithUtils::GetEfficiency(fileNames[nfile], objectNames[nobject]);
      TH1D* plot = JudithUtils::GenerateEfficiencyDistribution(eff);

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
        legend->AddEntry(plot, "MDBM-01, 1500e", "F");
      if (nfile == 1)
        legend->AddEntry(plot, "MDBM-02, 1100e", "F");
      if (nfile == 2)
        legend->AddEntry(plot, "MDBM-03, 1500e", "F");
    }
  }

  legend->Draw();
}
