#! /bin/bash

###WAVEFORM ANALYSIS: WF-Exponential or METHOD2
###
###either fast hits or all hits can be choosen for the analysis.








#DUT="./dut-280415-13-14-16-17-18-19-20-30-1-2-3-4"
RCE="/Volumes/Sonias_Backup/Testbeam_may2015/CosmicData/2015-05-15/run3/cosmic_000003"
ALIG=/Volumes/Sonias_Backup/Testbeam_may2015/CosmicData/2015-05-15/run2/cosmic_000002.root




echo "-------------------JUDITH VERSION 1.1 ---> chi2 align ON"
echo "0 TILT---MAY2015"

#echo "-------------------syncronization"
#./Judith -c synchronize -i $RCE.root -o $RCE-sync.root -I $DUT.root -O $DUT-sync.root -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg
echo "-------------------CoarseAlign telescope"
./Judith -c coarseAlign -i $ALIG -r configs/reforig_May2015_15deg.cfg -t configs/globalorig_May2015.cfg
echo "-------------------FineAlign telescope"
./Judith -c fineAlign -i $ALIG -r configs/reforig_May2015_15deg.cfg -t configs/globalorig_May2015.cfg -n 20000
#echo "-------------------CoarseAlign dut"
#./Judith -c coarseAlignDUT -i $RCE-sync.root -I $DUT-sync.root -r configs/reforig_May2015_0deg.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg
#echo "-------------------FineAlign dut"
#./Judith -c fineAlignDUT -i $RCE-sync.root -I $DUT-sync.root -t configs/globalorig.cfg -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg

echo "-------------------process telescope"
./Judith -c process -i $RCE.root -o $RCE-15deg-proc.root -r configs/reforig_May2015_15deg.cfg -t configs/globalorig_May2015.cfg -R $RCE-0deg-proc-result.root
#echo "-------------------process dut"
#./Judith -c process -i $DUT-sync.root -o $DUT-sync-proc.root -r configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-sync-proc-result.root
echo "-------------------Analysis telescope"
./Judith -c analysis -i $RCE-15deg-proc.root -r configs/reforig_May2015_15deg.cfg -t configs/globalorig_May2015.cfg -R $RCE-15deg-analysis-result.root
#echo "-------------------Analysis dut"
#./Judith -c analysisDUT -i $RCE-sync-proc.root -I $DUT-sync-proc.root -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-analysis-result.root

