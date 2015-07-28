{
  const char* fileNames[] =
    { "desy03/ref03-01-dut03-01-analysis.root" };

  const char* objectNames[] =
    { "Efficiency/DUTDiamondMap" };

  TFile* output = new TFile("MDBM01-1100e.root", "RECREATE");

  const unsigned int nfiles = sizeof(fileNames) / sizeof(const char*);
  const unsigned int nobjects = sizeof(objectNames) / sizeof(const char*);

  typedef TEfficiency* ObjType;
  ObjType object0 = 0;
  for (unsigned int nfile = 0; nfile < nfiles; nfile++)
  {
    TFile* f = new TFile(fileNames[nfile], "READ");
    for (unsigned int nobject = 0; nobject < nobjects; nobject++)
    {
      ObjType object = 0;
      f->GetObject(objectNames[nobject], object);
      if (!object) continue;
      object->SetDirectory(0);

      if (!object0)
        object0 = object;
      else 
        object0->Add(*object);
    }
    delete f;
  }

  TEfficiency* eff = object0;
  eff->SetDirectory(output);  

  output->Write();
  delete output;
}
