import math
import ROOT
from numpy import mean, sqrt, square
DEBUG=False
WAIT=False
REQUIRE_DUT_HIT=True
class Event:
    def __init__(self,telescope, x_truth, x_slope):
        self.telescope = telescope
        self.x_truth = x_truth
        self.x_slope = x_slope
        self.points = []
        self.slope = 0.0;
        self.slopeErr = 0.0;
        self.intercept = 0.;
        self.interceptErr = 0.;
        self.chi2 = 0.0;
        self.covariance = 0.0;
        
    def GetXTrue(self, plane):
        return self.x_truth + self.x_slope*plane.z
    
    def Digitize(self, r):

        for p in self.telescope:
            self.points +=[[p.ReturnPixelX(self.GetXTrue(p),r), p.pixelerr, p.z, p.mask]]
            if DEBUG:
                print 'Telescope: ',p.ReturnPixelX(self.GetXTrue(p),r)
                
    def Fit(self):
        if DEBUG:
            print ''
        self.slope = 0.;
        self.slopeErr = 0.;
        self.intercept = 0.;
        self.interceptErr = 0.;
        self.chi2 = 0.;
        self.covariance = 0.;
        err=[]
        
        # https://ned.ipac.caltech.edu/level5/Leo/Stats7_2.html
        A=0.0
        B=0.0
        C=0.0
        D=0.0
        E=0.0
        F=0.0;
        for p in self.points:
            # skip if masked
            if p[3]:
                continue
            # p is [x, xerr, z, mask]
            wt = 1.0 / p[1]**2
            # Check for divison by 0
            A += wt*p[2];
            B += wt;
            C += wt*p[0];
            D += wt*p[2]**2;
            E += wt*p[2]*p[0];
            F += wt*p[0]**2;
            
        DEN = D*B-A*A;
        if DEN>0.0:
            self.slope        = (E*B - C*A)/DEN;
            self.intercept    = (D*C - E*A) / DEN;
            self.slopeErr     = math.sqrt(B/DEN);
            self.interceptErr = math.sqrt(D/DEN);
            self.covariance   = -A / DEN;
            
    def Residual(self, nplane=2):
        if nplane>len(self.telescope):
            print 'ERROR plane too large'
        #if self.telescope[nplane].mask:
        #    print 'Point: ',self.points[nplane][0],' fit: ',(self.slope*self.telescope[nplane].z+self.intercept)
        return self.points[nplane][0]-(self.slope*self.telescope[nplane].z+self.intercept)

    def TruthResidual(self, nplane=2):
        if nplane>len(self.telescope):
            print 'ERROR plane too large'
        #print 'Results: ',(self.GetXTrue(self.telescope[nplane]) - self.telescope[nplane].x - self.telescope[nplane].pixelwidth/2.0)
        #print 'Truth: ',self.GetXTrue(self.telescope[nplane]),' x: ',self.telescope[nplane].x,' width: ',self.telescope[nplane].pixelwidth/2.0
        return (self.GetXTrue(self.telescope[nplane]) - self.telescope[nplane].x - self.telescope[nplane].pixelwidth/2.0)

class Plane:
    #100,445,545
    def __init__(self, x, z, pixelwidth=50.0, mask=False, npixel=12, allhits=False):

        self.x=x
        self.z=z
        self.pixelwidth=pixelwidth
        self.pixelerr=self.pixelwidth / math.sqrt(12.0)
        self.npixel=npixel
        self.mask=mask
        self.allhits = allhits

    def ReturnPixel(self, x_truth, r):
        # return pixel location - self.pixelwidth/2.0
        n_pixel = round((x_truth - self.x - self.pixelwidth/2.0)/self.pixelwidth )
        #if self.npixel<abs(n_pixel):
        if REQUIRE_DUT_HIT and not self.allhits:
            if self.npixel==1:
                if n_pixel==0:
                    return n_pixel
                else:
                    return -111
            else:
                return n_pixel
        else:
            if self.npixel==1:
                return 0
            else:
                return n_pixel
        #return round((x_truth - self.x)/self.pixelwidth )
        #return round((x_truth-self.x + r.Gaus(0.0,15.0))/self.pixelwidth)

    def ReturnPixelX(self, x_truth, r):
                    
        return float(self.ReturnPixel(x_truth,r))*self.pixelwidth + self.x + self.pixelwidth/2.0

NEVENTS=100000
DUT=True
dut_offset=12.0/2.0*50.0+20.0
slope_spread = 545.0*math.tan(0.000185)
r = ROOT.TRandom3()
r.SetSeed(5)
can = ROOT.TCanvas()
truth_residual = ROOT.TH1F('truth_residual','truth_residual',2000,-100.0,100.0)
#for offset in [[0.0, 15.0, 0.0, -4.5, 10.0]]:
for offset in [[0.0,-40.0, -1.0, 50.5, 12.0]]:

    telescope = []
    hists  = []
    if False:
        hists += [ROOT.TH1F('plane0','plane0',1000,-100.0,100.0)]
        hists += [ROOT.TH1F('plane1','plane1',1000,-100.0,100.0)]
        hists += [ROOT.TH1F('plane2','plane2',1000,-100.0,100.0)]
        #hists += [ROOT.TH1F('plane3','plane3',1000,-100.0,100.0)]
        #hists += [ROOT.TH1F('plane4','plane4',1000,-100.0,100.0)]        
    
        # Generate Telescope
        telescope += [ Plane(0.0,50.0)      ]
        telescope += [ Plane(offset[0],445.0)]
        telescope += [ Plane(offset[1],545.0)]
        #telescope += [ Plane(offset[2],645.0)]
        #telescope += [ Plane(offset[3],745.0)]
    else:
    
        hists += [ROOT.TH1F('plane0','plane0',200,-500.0,500.0)]
        hists += [ROOT.TH1F('plane1','plane1',100,-100.0,100.0)]
        hists += [ROOT.TH1F('plane2','plane2',200,-500.0,500.0)]
        if DUT:
            hists += [ROOT.TH1F('dut','dut',2000,-100.0,100.0)]        
        hists += [ROOT.TH1F('plane3','plane3',100,-100.0,100.0)]
        hists += [ROOT.TH1F('plane4','plane4',200,-500.0,500.0)]        
        hists += [ROOT.TH1F('plane5','plane5',100,-100.0,100.0)]        
        if DUT:
            hists += [ROOT.TH1F('dut_den','dut_den',2000,-100.0,100.0)]            
        # Generate Telescope
        telescope += [ Plane(0.0,      0.0, pixelwidth=250.0)]
        telescope += [ Plane(offset[0],50.0, pixelwidth=50.0)]
        telescope += [ Plane(offset[1],100.0, pixelwidth=250.0)]
        if DUT:
            telescope += [ Plane(dut_offset, 271.5, pixelwidth=33.0, mask=True, npixel=1)]        
        telescope += [ Plane(offset[2],445.0, pixelwidth=50.0)]
        telescope += [ Plane(offset[3],495.0, pixelwidth=250.0)]
        telescope += [ Plane(offset[4],545.0, pixelwidth=50.0)]
        if DUT: # must match above plane
            telescope += [ Plane(dut_offset, 271.5, pixelwidth=33.0, mask=True, npixel=1, allhits=True)]                
              
    events=[]
    
    for i in range(0,NEVENTS):
        if i%5000==0:
            print 'Event: ',i
        x_truth = r.Rndm()*9.0*50.0+75.0
        x_slope = r.Gaus(0.0,slope_spread)
        
        events+=[Event(telescope,x_truth,x_slope)]
        events[i].Digitize(r)
        events[i].Fit()
        for hh in range(0,len(hists)):
            hists[hh].Fill(events[i].Residual(hh))
            if DUT and events[i].telescope[hh].mask:
                truth_residual.Fill(events[i].TruthResidual(hh))
        #print x_truth

    can.cd()
    for hh in range(0,len(hists)):
        hists[hh].Draw()
        can.Update()
        if WAIT:
            can.WaitPrimitive()

    truth_residual.Draw()
    can.Update()
    if WAIT:
        can.WaitPrimitive()

    # find the x position
    low_bin=-1
    high_bin=-1
    # Integrated beyond edge for efficiency
    extra_range = 0
    # central efficiency
    #central_eff = 0.44
    central_eff = 0.89
    for i in range(0,truth_residual.GetNbinsX()):
        if truth_residual.GetXaxis().GetBinLowEdge(i)>=-(central_eff*telescope[3].pixelwidth/2.0) and low_bin<0:
            low_bin=i
        if truth_residual.GetXaxis().GetBinUpEdge(i)>=(central_eff*telescope[3].pixelwidth/2.0):
            high_bin=i
            break
    print 'High Bin: ',high_bin,' Low: ',low_bin
    print 'Truth Hits: ',truth_residual.Integral(low_bin,high_bin)
    
    if DUT:
        hits_dut = hists[3]
        den_dut = hists[7]
        dut_list=[]
        print 'Centered at 0 in residuals : Reco Hits: ',hits_dut.Integral(low_bin,high_bin)
        print 'Reco / Truth: ',hits_dut.Integral(low_bin,high_bin)/truth_residual.Integral(low_bin,high_bin)
        for i in range(-200,200):
            tmp_low_bin = low_bin+i
            if tmp_low_bin<1:
                tmp_low_bin=1
            dut_list+=[hits_dut.Integral(tmp_low_bin,high_bin+i)]
            #print '   vary: ',i,' ',hits_dut.Integral(low_bin+i,high_bin+i)

        dut_rms=[]
        my_mean = mean(dut_list)
        for m in dut_list:
            dut_rms+=[m-my_mean]
        print 'MAX Hits from reco residuals using truth hits: ',max(dut_list),' +/- ',sqrt(mean(square(dut_rms))),' mean: ',my_mean


        eff_dut = den_dut.Clone()
        for i in range(-1,hits_dut.GetNbinsX()+1):
            eff_dut.SetBinContent(i,0.0)
        range_bins = high_bin-low_bin
        eff_list = []
        
        for i in range(1,hits_dut.GetNbinsX()-range_bins):
            if den_dut.Integral(i,range_bins+i)>0.0:
                tmp_i=i-extra_range
                if tmp_i<1:
                    tmp_i=1
                eff = hits_dut.Integral(tmp_i,range_bins+i+extra_range)/den_dut.Integral(i,range_bins+i)
                if eff>0.0:
                    #print 'Eff: ',i,' ',eff
                    eff_list+=[eff]
                eff_dut.SetBinContent(i+int(range_bins/2.0),eff)

        hits_dut.Draw()
        if WAIT:
            can.Update()
            can.WaitPrimitive()
        eff_dut.Draw()
        print 'MAX Eff: ',max(eff_list)
        eff_dut.GetYaxis().SetRangeUser(0,1.2)
        eff_dut.GetYaxis().SetTitle('DUT Efficiency')
        eff_dut.GetXaxis().SetTitle('Center of Pixel Position [#mum]')                
        can.Update()
        can.WaitPrimitive()
        
print 'DONE'
