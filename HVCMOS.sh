#! /bin/bash

export PATH="HVCMOSData"

# Alignment
export ALIG=$PATH/Cosmic.root

# Setpoint specific
export DUT=$PATH/DUT
export RCE=$PATH/Cosmic
export RCE_MASKED=$PATH/Cosmic-mask
export RCE_PROCC=$PATH/Cosmic-process
export DUT_PROCC=$PATH/DUT-process

echo "HVCMOS Efficiency Analysis --- 0 TILT"

echo "-------------------Apply mask"
./Judith -c applyMask -i $RCE.root -o ${RCE_MASKED}.root  -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg


echo "-------------------syncronization"
./Judith -c synchronizeRMS -i ${RCE_MASKED}.root -o ${RCE_MASKED}_sync.root -I $DUT.root -O ${DUT}_sync.root -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -d configs/DUT-HVCMOS.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg -s 0 #-n 300000

echo "-------------------CoarseAlign telescope"
./Judith -c coarseAlign -i $ALIG -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg
echo "-------------------FineAlign telescope"
./Judith -c fineAlign -i $ALIG -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg -n 20000
#
## change to the sync names
export RCE_MASKED=${RCE_MASKED}_sync
export DUT=${DUT}_sync
echo "-------------------CoarseAlign dut"
./Judith -c coarseAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig.cfg -d configs/DUT-HVCMOS.cfg
#echo "-------------------FineAlign dut"
##./Judith -c fineAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -t configs/globalorig.cfg -r configs/reforig_July2015_0deg.cfg -d configs/DUT-HVCMOS.cfg


echo "-------------------process telescope"
./Judith -c process -i ${RCE_MASKED}.root -o ${RCE_PROCC}.root -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg -R ${RCE}-proccess-result.root
echo "-------------------process dut"
./Judith -c process -i ${DUT}.root -o ${DUT_PROCC}.root -r configs/DUT-HVCMOS.cfg -t configs/globalorig.cfg -R ${DUT}-procress-result.root
echo "-------------------Analysis telescope"
./Judith -c analysis -i ${RCE_PROCC}.root -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg -R ${RCE}-analysis-result.root
echo "-------------------Analysis dut"
./Judith -c analysisDUT -i ${RCE_PROCC}.root -I ${DUT_PROCC}.root -r configs/reforig_HVCMOS-v4_36-60v-20150710.cfg -d configs/DUT-HVCMOS.cfg -t configs/globalorig_HVCMOS-v4_36-60v-20150710.cfg -R ${DUT}-analysis-result.root

