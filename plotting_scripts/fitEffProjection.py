import ROOT

#-----------------------------------------------------------------------------
# Load necessary shared libraries
#
def setPlotDefaults(root, options = None):

    #root.gROOT.SetStyle('Plain')

    root.gStyle.SetFillColor(10)           
    root.gStyle.SetFrameFillColor(10)      
    root.gStyle.SetCanvasColor(10)         
    root.gStyle.SetPadColor(10)            
    root.gStyle.SetTitleFillColor(0)       
    root.gStyle.SetStatColor(10)   
    
    root.gStyle.SetCanvasBorderMode(0)
    root.gStyle.SetFrameBorderMode(0) 
    root.gStyle.SetPadBorderMode(0)   
    root.gStyle.SetDrawBorder(0)      
    root.gStyle.SetTitleBorderSize(0)
    
    root.gStyle.SetFuncWidth(2)
    root.gStyle.SetHistLineWidth(2)
    root.gStyle.SetFuncColor(2)
    
    root.gStyle.SetPadTopMargin(0.08)
    root.gStyle.SetPadBottomMargin(0.16)
    root.gStyle.SetPadLeftMargin(0.16)
    root.gStyle.SetPadRightMargin(0.12)
  
    # set axis ticks on top and right
    root.gStyle.SetPadTickX(1)         
    root.gStyle.SetPadTickY(1)         
  
    # Set the background color to white
    root.gStyle.SetFillColor(10)           
    root.gStyle.SetFrameFillColor(10)      
    root.gStyle.SetCanvasColor(10)         
    root.gStyle.SetPadColor(10)            
    root.gStyle.SetTitleFillColor(0)       
    root.gStyle.SetStatColor(10)           
  
  
    # Turn off all borders
    root.gStyle.SetCanvasBorderMode(0)
    root.gStyle.SetFrameBorderMode(0) 
    root.gStyle.SetPadBorderMode(0)   
    root.gStyle.SetDrawBorder(0)      
    root.gStyle.SetTitleBorderSize(0) 
  
    # Set the size of the default canvas
    root.gStyle.SetCanvasDefH(400)          
    root.gStyle.SetCanvasDefW(650)          
    #gStyle->SetCanvasDefX(10)
    #gStyle->SetCanvasDefY(10)   
  
    # Set fonts
    font = 42
    #root.gStyle.SetLabelFont(font,'xyz')
    #root.gStyle.SetStatFont(font)       
    #root.gStyle.SetTitleFont(font)      
    #root.gStyle.SetTitleFont(font,'xyz')
    #root.gStyle.SetTextFont(font)       
    #root.gStyle.SetTitleX(0.3)        
    #root.gStyle.SetTitleW(0.4)        
  
   # Set Line Widths
   #gStyle->SetFrameLineWidth(0)
   #root.gStyle.SetFuncWidth(2)
   #root.gStyle.SetHistLineWidth(2)
   #root.gStyle.SetFuncColor(2)
   #
   # Set tick marks and turn off grids
    root.gStyle.SetNdivisions(505,'xyz')
   #
   # Set Data/Stat/... and other options
   #root.gStyle.SetOptDate(0)
   #root.gStyle.SetDateX(0.1)
   #root.gStyle.SetDateY(0.1)
   #gStyle->SetOptFile(0)
   ##root.gStyle.SetOptStat(1110)
    root.gStyle.SetOptStat('reimo')
    root.gStyle.SetOptFit(111)
    root.gStyle.SetStatFormat('4.3f')
    root.gStyle.SetFitFormat('4.3f')
   #gStyle->SetStatTextColor(1)
   #gStyle->SetStatColor(1)
   #gStyle->SetOptFit(1)
   #gStyle->SetStatH(0.20)
   #gStyle->SetStatStyle(0)
   #gStyle->SetStatW(0.30)
   #gStyle -SetStatLineColor(0)
   #root.gStyle.SetStatX(0.919)
   #root.gStyle.SetStatY(0.919)
   #root.gStyle.SetOptTitle(0)
   #gStyle->SetStatStyle(0000)    # transparent mode of Stats PaveLabel
   #root.gStyle.SetStatBorderSize(0)
   #
    #root.gStyle.SetLabelSize(0.065,'xyz')
    #gStyle -> SetLabelOffset(0.005,'xyz')
    #root.gStyle.SetTitleY(.5)
    root.gStyle.SetTitleOffset(1.0,'xz')
    root.gStyle.SetTitleOffset(1.1,'y')
    root.gStyle.SetTitleSize(0.065, 'xyz')
    root.gStyle.SetLabelSize(0.065, 'xyz')
    #root.gStyle.SetTextAlign(22)
    root.gStyle.SetTextSize(0.1)
   #
   ##root.gStyle.SetPaperSize(root.TStyle.kA4)  
    root.gStyle.SetPalette(1)
   #
   ##root.gStyle.SetHistMinimumZero(True)
   
    root.gROOT.ForceStyle()
#-----------------------------------------
def Format(h):

    h.SetLineColor(1)
    h.SetMarkerColor(1)
    h.SetTitle('Projection of 2D Efficiency')
#-----------------------------------------
def Style():
    ROOT.gROOT.LoadMacro('/Users/schae/testarea/CAFAna/HWWMVACode/atlasstyle-00-03-05/AtlasStyle.C')                   
    ROOT.gROOT.LoadMacro('/Users/schae/testarea/CAFAna/HWWMVACode/atlasstyle-00-03-05/AtlasUtils.C')
    ROOT.SetAtlasStyle()

def Fit(file_name='./../Judith_original/TestData/dut-120V_runs-23-24-25-26-27-28-29-30-1-2-3-4-5-6-7_settings1_sync_analysis-result-cutslope-align.root',_suffix=''):
    rebin=2

#Fit('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result-cutt0wider-align.root','60V_cutt0')
#Fit('./../Judith_original/TestData/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings_sync_analysis-result-cutt0wider-align.root','90V_cutt0')
#Fit('./../Judith_original/TestData/dut-120V_runs-23-24-25-26-27-28-29-30-1-2-3-4-5-6-7_settings1_sync_analysis-result-cutt0wider-align.root','120V_cutt0')
#
#Fit('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result-nocut-align.root','60V_nocut')
#Fit('./../Judith_original/TestData/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings_sync_analysis-result-nocut-align.root','90V_nocut')
#Fit('./../Judith_original/TestData/dut-120V_runs-23-24-25-26-27-28-29-30-1-2-3-4-5-6-7_settings1_sync_analysis-result-nocut-align.root','120V_nocut')
#
#Fit('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result-cutslope-align.root','60V_cutslope')
#Fit('./../Judith_original/TestData/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings_sync_analysis-result-cutslope-align.root','90V_cutslope')
#Fit('./../Judith_original/TestData/dut-120V_runs-23-24-25-26-27-28-29-30-1-2-3-4-5-6-7_settings1_sync_analysis-result-cutslope-align.root','120V_cutslope')

    
    #f = ROOT.TFile.Open('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result.root')
    #f = ROOT.TFile.Open('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result-cutslope-align.root')
    #f = ROOT.TFile.Open('./../Judith_original/TestData/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings_sync_analysis-result-cutslope-align.root')
    f = ROOT.TFile.Open(file_name)        
    twoD = f.Get('Efficiency/sensor0_TrackResEffFine')
    
    hx = twoD.ProjectionX("px")
    hy = twoD.ProjectionY("py")
    hx.Rebin(rebin)
    hy.Rebin(rebin)

    hx.Fit('gaus')
    hy.Fit('gaus')
    
    #hx.SetStats(True)
    #hy.SetStats(True)
      
    hx.GetXaxis().SetTitle('x Projection of Efficiency [#mum]')
    hy.GetXaxis().SetTitle('y Projection of Efficiency [#mum]')
    hx.GetYaxis().SetTitle('Arbitrary Units')
    hy.GetYaxis().SetTitle('Arbitrary Units')
    hy.GetXaxis().SetRangeUser(-100,100)
    hx.GetXaxis().SetRangeUser(-100,100)
    Format(hx)
    Format(hy)
    can = ROOT.TCanvas("c2","c2",100,10,800,600);
    can.cd()
    #can.SetStats(1111)
    hx.SetTitle('')
    hx.Draw()
    can.Update()
    #can.WaitPrimitive()
    can.SaveAs('projectionx_'+_suffix+'.eps')
    can.SaveAs('projectionx_'+_suffix+'.pdf')
    can.SaveAs('projectionx_'+_suffix+'.C')        


    hy.Draw()
    can.Update()
    #can.WaitPrimitive()
    can.SaveAs('projectiony_'+_suffix+'.eps')
    can.SaveAs('projectiony_'+_suffix+'.pdf')
    can.SaveAs('projectiony_'+_suffix+'.C')

#Style()
setPlotDefaults(ROOT)
Fit('./../Judith_original/TestData/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1_sync_analysis-result-nocut-align.root','nocut_60V')
Fit('./../Judith_original/TestData/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings_sync_analysis-result-nocut-align.root','nocut_90V')
Fit('./../Judith_original/TestData/dut-120V_runs-23-24-25-26-27-28-29-30-1-2-3-4-5-6-7_settings1_sync_analysis-result-nocut-align.root','nocut_120V')
