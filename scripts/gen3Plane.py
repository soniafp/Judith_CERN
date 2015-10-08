import math
import ROOT
DEBUG=False
class Event:
    def __init__(self,telescope, x_truth):
        self.telescope=telescope
        self.x_truth=x_truth
        self.points = []
        self.slope = 0.0;
        self.slopeErr = 0.0;
        self.intercept = 0.;
        self.interceptErr = 0.;
        self.chi2 = 0.0;
        self.covariance = 0.0;
    def Digitize(self, r):

        for p in self.telescope:
            self.points +=[[p.ReturnPixelX(self.x_truth,r),p.pixelerr,p.z]]
            if DEBUG:
                print 'Telescope: ',p.ReturnPixelX(self.x_truth,r)
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
            # p is [x, xerr, z]
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
            self.slope = (E*B - C*A)/DEN;
            self.intercept = (D*C - E*A) / DEN;
            self.slopeErr = math.sqrt(B/DEN);
            self.interceptErr = math.sqrt(D/DEN);
            self.covariance = -A / DEN;
            
    def Residual(self, nplane=2):
        if nplane>len(self.telescope):
            print 'ERROR plane too large'
        return self.points[nplane][0]-(self.slope*self.telescope[nplane].z+self.intercept)

class Plane:
    #100,445,545
    def __init__(self, x, z, pixelwidth=50.0):

        self.x=x
        self.z=z       
        self.pixelwidth=pixelwidth
        self.pixelerr=self.pixelwidth/math.sqrt(12.0)
        self.npixel=12

    def ReturnPixel(self,x_truth,r):
        # return pixel location - self.pixelwidth/2.0 
        return round((x_truth - self.x)/self.pixelwidth )
        #return round((x_truth-self.x + r.Gaus(0.0,7.0))/self.pixelwidth)

    def ReturnPixelX(self,x_truth, r):

        return float(self.ReturnPixel(x_truth,r))*self.pixelwidth + self.x + self.pixelwidth/2.0
            

NEVENTS=40000
r = ROOT.TRandom3()
r.SetSeed(5)
can = ROOT.TCanvas()
for offset in [[0.0, 15.0, 0.0, -4.5, 10.0]]:

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
    
        hists += [ROOT.TH1F('plane0','plane0',1000,-500.0,500.0)]
        hists += [ROOT.TH1F('plane1','plane1',1000,-100.0,100.0)]
        hists += [ROOT.TH1F('plane2','plane2',1000,-500.0,500.0)]
        hists += [ROOT.TH1F('plane3','plane3',1000,-100.0,100.0)]
        hists += [ROOT.TH1F('plane4','plane4',1000,-500.0,500.0)]        
        hists += [ROOT.TH1F('plane5','plane5',1000,-100.0,100.0)]        
    
        # Generate Telescope
        telescope += [ Plane(0.0,0.0, pixelwidth=250.0)      ]
        telescope += [ Plane(offset[0],50.0, pixelwidth=50.0)]
        telescope += [ Plane(offset[1],100.0, pixelwidth=250.0)]
        telescope += [ Plane(offset[2],445.0, pixelwidth=50.0)]
        telescope += [ Plane(offset[3],495.0, pixelwidth=250.0)]
        telescope += [ Plane(offset[4],545.0, pixelwidth=50.0)]
              
    events=[]
    
    for i in range(0,NEVENTS):
        if i%500==0:
            print 'Event: ',i
        x_truth = r.Rndm()*9.0*50.0+75.0
        events+=[Event(telescope,x_truth)]
        events[i].Digitize(r)
        events[i].Fit()
        for hh in range(0,len(hists)):
            hists[hh].Fill(events[i].Residual(hh))
        #print x_truth


    can.cd()
    for hh in range(0,len(hists)):
        hists[hh].Draw()
        can.Update()
        can.WaitPrimitive()

print 'DONE'
