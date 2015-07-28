#! /bin/bash

###WAVEFORM ANALYSIS: WF-Exponential or METHOD2
###
###either fast hits or all hits can be choosen for the analysis.







DUT=./TestData/testbeam_50pixel_unirr_run24
RCE=./TestData/cosmic_000024
RCE_MASKED=./TestData/cosmic_000024-mask
ALIG=./TestData/cosmic_000024.root

RCE_PROCC=./TestData/cosmic_000024-process
DUT_PROCC=./TestData/testbeam_50pixel_unirr_run24-proccess




echo "15 TILT---JUL2015 ----XFAB -------SONIA"

echo "-------------------Apply mask"
./Judith -c applyMask -i $RCE.root -o ${RCE_MASKED}.root  -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg


#echo "-------------------syncronization"
#./Judith -c synchronize -i $RCE.root -o $RCE_sync.root -I $DUT.root -O $DUT_sync.root -r configs/reforig_July2015_15deg.cfg -d configs/dutXfab.cfg -t configs/globalorig_July2015.cfg -s 4
echo "-------------------CoarseAlign telescope"
./Judith -c coarseAlign -i $ALIG -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg
echo "-------------------FineAlign telescope"
./Judith -c fineAlign -i $ALIG -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -n 20000
echo "-------------------CoarseAlign dut"
./Judith -c coarseAlignDUT -i ${RCE_MASKED}.root -I $DUT.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg
echo "-------------------FineAlign dut"
./Judith -c fineAlignDUT -i $RCE_sync.root -I $DUT_sync.root -t configs/globalorig.cfg -r configs/reforig_July2015_0deg.cfg -d configs/dutXfab.cfg

echo "-------------------process telescope"
./Judith -c process -i ${RCE_MASKED}.root -o ${RCE_PROCC}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -R ${RCE}_15deg-proccess-result.root
echo "-------------------process dut"
./Judith -c process -i $DUT.root -o ${DUT_PROCC}.root -r configs/dutXfab.cfg -t configs/globalorig.cfg -R ${DUT}_15deg-procress-result.root
echo "-------------------Analysis telescope"
./Judith -c analysis -i ${RCE_PROCC}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -R ${RCE}_15deg-analysis-result.root
echo "-------------------Analysis dut"
./Judith -c analysisDUT -i ${RCE_PROCC}.root -I ${DUT_PROCC}.root -r configs/reforig_July2015_15deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg -R ${DUT}_analysis-result.root

