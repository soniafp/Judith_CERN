#! /bin/bash

###WAVEFORM ANALYSIS: WF-Exponential or METHOD2
###
###either fast hits or all hits can be choosen for the analysis.








#DUT="./dut-280415-13-14-16-17-18-19-20-30-1-2-3-4"
RCE="/Volumes/Sonias_Backup/Testbeam_may2015/CosmicData/2015-05-18/run13/cosmic_000013"
ALIG=/Volumes/Sonias_Backup/Testbeam_may2015/CosmicData/2015-05-18/run14/cosmic_000014.root




echo "-------------------JUDITH VERSION 1.1 ---> chi2 align ON"
echo "0 TILT---MAY2015"

#echo "-------------------syncronization"
#./Judith -c synchronize -i $RCE.root -o $RCE-sync.root -I $DUT.root -O $DUT-sync.root -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg
echo "-------------------CoarseAlign telescope"
./Judith -c coarseAlign -i $ALIG -r configs/reforig_May2015_30deg.cfg -t configs/globalorig_May2015.cfg
echo "-------------------FineAlign telescope"
./Judith -c fineAlign -i $ALIG -r configs/reforig_May2015_30deg.cfg -t configs/globalorig_May2015.cfg -n 20000
#echo "-------------------CoarseAlign dut"
#./Judith -c coarseAlignDUT -i $RCE-sync.root -I $DUT-sync.root -r configs/reforig_May2015_0deg.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg
#echo "-------------------FineAlign dut"
#./Judith -c fineAlignDUT -i $RCE-sync.root -I $DUT-sync.root -t configs/globalorig.cfg -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg

echo "-------------------process telescope"
./Judith -c process -i $RCE.root -o $RCE-30deg-proc.root -r configs/reforig_May2015_30deg.cfg -t configs/globalorig_May2015.cfg -R $RCE-30deg-proc-result.root
#echo "-------------------process dut"
#./Judith -c process -i $DUT-sync.root -o $DUT-sync-proc.root -r configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-sync-proc-result.root
echo "-------------------Analysis telescope"
./Judith -c analysis -i $RCE-30deg-proc.root -r configs/reforig_May2015_30deg.cfg -t configs/globalorig_May2015.cfg -R $RCE-30deg-analysis-result.root
#echo "-------------------Analysis dut"
#./Judith -c analysisDUT -i $RCE-sync-proc.root -I $DUT-sync-proc.root -r configs/reforig_May2015_0deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg -R $DUT-analysis-result.root

