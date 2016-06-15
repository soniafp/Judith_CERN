import ROOT

can = ROOT.TCanvas()
f = ROOT.TFile.Open('quick.root','recreate')

hbox=ROOT.TH1F('box','',100,-50.0,50.0)
hsmear=ROOT.TH1F('smear','',100,-50.0,50.0)

r = ROOT.TRandom3();
r.SetSeed(5)
NEVT=1000000
w=1.0/float(NEVT)*50.0
for e in range(0,NEVT): #for i in range(25,75):
    a=r.Rndm()*50.0 - 25.0
    #if round(a)==0:
    #    print a, round(a)
    hbox.Fill(round(a),w)
    a+=r.Gaus(0.0,7.0)    
    hsmear.Fill(round(a),w)

can.cd()
hbox.Draw()
hsmear.SetLineColor(2)
hsmear.SetMarkerColor(2)
hsmear.Draw('same')
can.Update()
can.WaitPrimitive()

hsmear.SetDirectory(f)
hbox.SetDirectory(f)

print 'eff for central 80%: ',hsmear.Integral(47,66)/hbox.Integral(47,66)

f.Write()
f.Close()
