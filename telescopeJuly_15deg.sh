#! /bin/bash

###WAVEFORM ANALYSIS: WF-Exponential or METHOD2
###
###either fast hits or all hits can be choosen for the analysis.

export PATH="./../Judith_CERN/TestData"
#export PATH="./../Judith_original/TestData"

# Alignment
#export ALIG=$PATH/cosmic_000024.root
export ALIG=$PATH/cosmic_000112.root
#export ALIG_MASKED=$PATH/cosmic_000112-align.root
#export ALIG=$PATH/cosmic-120V_runs-23-24-25-26-27-28-29-30.root

# 120 V
#export RCE=$PATH/cosmic_000112
export DUT=$PATH/dut-120V_runs-23-24-25-26-27-28-29-30
export RCE=$PATH/cosmic-120V_runs-23-24-25-26-27-28-29-30
#export RCE=$PATH/cosmic_000024
export RCE_MASKED=$PATH/cosmic-120V_runs-23-24-25-26-27-28-29-30-mask
#export RCE_MASKED=$PATH/cosmic_000024-MASKED
export RCE_PROCC=$PATH/cosmic-120V_runs-23-24-25-26-27-28-29-30-process
#export RCE_PROCC=$PATH/testbeam_50pixel_unirr_run24-process
#export RCE_PROCC=$PATH/testbeam_50pixel_unirr_run24-proccess
export DUT_PROCC=$PATH/dut-120V_runs-23-24-25-26-27-28-29-30-proccess

# 90 V
export DUT=$PATH/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings
export RCE=$PATH/cosmic-90V_runs-9-11-1-2-3-5-6-8-9
export RCE_MASKED=$PATH/cosmic-90V_runs-9-11-1-2-3-5-6-8-9-mask
export RCE_PROCC=$PATH/cosmic-90V_runs-9-11-1-2-3-5-6-8-9-process
export DUT_PROCC=$PATH/dut-90V_runs-9-11-1-2-3-5-6-8-9_settings-process

# 60 V
#export DUT=$PATH/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1
#export RCE=$PATH/cosmic-60V_runs-2-3-4-5-6-7-1-4-5-6
#export RCE_MASKED=$PATH/cosmic-60V_runs-2-3-4-5-6-7-1-4-5-6-mask
#export RCE_PROCC=$PATH/cosmic-60V_runs-2-3-4-5-6-7-1-4-5-6-process
#export DUT_PROCC=$PATH/dut-60V_runs-2-3-4-5-6-7-1-4-5-6_settings1-process


echo "15 TILT---JUL2015 ----XFAB -------SONIA"

echo "-------------------Apply mask"
./Judith -c applyMask -i $RCE.root -o ${RCE_MASKED}.root  -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg


echo "-------------------syncronization"
./Judith -c synchronizeRMS -i ${RCE_MASKED}.root -o ${RCE_MASKED}_sync.root -I $DUT.root -O ${DUT}_sync.root -r configs/reforig_July2015_15deg.cfg -d configs/dutXfab.cfg -t configs/globalorig_July2015.cfg -s 0 #-n 300000
##echo "-------------------CoarseAlign telescope"
##./Judith -c coarseAlign -i $ALIG -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg
##echo "-------------------FineAlign telescope"
##./Judith -c fineAlign -i $ALIG -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -n 20000
#
## change to the sync names
export RCE_MASKED=${RCE_MASKED}_sync
export DUT=${DUT}_sync
echo "-------------------CoarseAlign dut"
##./Judith -c coarseAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg -y $1 -s $2
./Judith -c coarseAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig.cfg -d configs/dutXfab.cfg 
#
#echo "-------------------FineAlign dut"
##./Judith -c fineAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -t configs/globalorig.cfg -r configs/reforig_July2015_15deg.cfg -d configs/dutXfab.cfg
echo "-------------------process telescope"
./Judith -c process -i ${RCE_MASKED}.root -o ${RCE_PROCC}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -R ${RCE}_15deg-proccess-result.root
echo "-------------------process dut"
./Judith -c process -i ${DUT}.root -o ${DUT_PROCC}.root -r configs/dutXfab.cfg -t configs/globalorig.cfg -R ${DUT}_15deg-procress-result.root
echo "-------------------Analysis telescope"
./Judith -c analysis -i ${RCE_PROCC}.root -r configs/reforig_July2015_15deg.cfg -t configs/globalorig_July2015.cfg -R ${RCE}_15deg-analysis-result.root
echo "-------------------Analysis dut"
./Judith -c analysisDUT -i ${RCE_PROCC}.root -I ${DUT_PROCC}.root -r configs/reforig_July2015_15deg.cfg -d configs/dutXfab.cfg -t configs/globalorig.cfg -R ${DUT}_analysis-result.root

