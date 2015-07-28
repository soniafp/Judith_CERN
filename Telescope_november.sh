#! /bin/bash

###WAVEFORM ANALYSIS: WF-Exponential or METHOD2
###
###either fast hits or all hits can be choosen for the analysis.


#then

#echo "-------------------WF_METHOD 2 -----all hits"

#DUT="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/dut/method2_thr6-3/dut-13-14-16-17-18-19-20-30-1-2-3-4"
#RCE="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/method2_thr6-3/cosmic-13-14-16-17-18-19-20-30-1-2-3-4"
#ALIG=/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/method2_thr6-3/2014-11-22_run11.root




echo "-------------------WF_EXPONENTIAL -----only fast hits"
echo "-------------------WF_EXPONENTIAL -----only fast hits"

DUT="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/dut/exponential/WF_only_fast/dut-280415-13-14-16-17-18-19-20-30-1-2-3-4"
RCE="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/exponential/WF_only_fast/cosmic-13-14-16-17-18-19-20-30-1-2-3-4"
ALIG=/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/exponential/WF_only_fast/2014-11-22_run11.root


#echo "-------------------WF_EXPONENTIAL -----all hits"

#DUT="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/dut/exponential/WF_all/dut-280415-13-14-16-17-18-19-20-30-1-2-3-4"
#RCE="/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/exponential/WF_all/cosmic-13-14-16-17-18-19-20-30-1-2-3-4"

#ALIG=/Users/Administrador/Documents/MAPS/XTB01/testbeams_analysis/SPSnovember/JudithFEI4_V2/cosmic/exponential/WF_all/2014-11-22_run11.root

echo "-------------------JUDITH VERSION 2"

echo "-------------------syncronization"
./Judith -c synchronize -i $RCE.root -o $RCE-sync.root -I $DUT.root -O $DUT-sync.root -r configs/reforig.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg
#echo "-------------------CoarseAlign telescope"
#./Judith -c coarseAlign -i $ALIG -r configs/reforig.cfg -t configs/globalorig.cfg
#echo "-------------------FineAlign telescope"
#./Judith -c chi2Align -i $ALIG -r configs/reforig.cfg -t configs/globalorig.cfg -n 20000
#echo "-------------------CoarseAlign dut"
#./Judith -c coarseAlignDUT -i $RCE-sync.root -I $DUT-sync.root -r configs/reforig.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg
#echo "-------------------FineAlign dut"
#./Judith -c fineAlignDUT -i $RCE-sync.root -I $DUT-sync.root -t configs/globalorig.cfg -r configs/reforig.cfg -d configs/dutXfab.cfg

#echo "-------------------process telescope"
#./Judith -c process -i $RCE-sync.root -o $RCE-sync-proc.root -r configs/reforig.cfg -t configs/globalorig.cfg -R $RCE-sync-proc-result.root
#echo "-------------------process dut"
#./Judith -c process -i $DUT-sync.root -o $DUT-sync-proc.root -r configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-sync-proc-result.root
#echo "-------------------Analysis telescope"
#./Judith -c analysis -i $RCE-sync-proc.root -r configs/reforig.cfg -t configs/globalorig.cfg -R $RCE-analysis-result.root
#echo "-------------------Analysis dut"
#./Judith -c analysisDUT -i $RCE-sync-proc.root -I $DUT-sync-proc.root -r configs/reforig.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-analysis-result.root

