#! /bin/bash

export PATH="TowerJazz"

# Alignment
export ALIG=$PATH/cosmic_000768_000000.root

# Setpoint specific
#export DUT=$PATH/DUT
##export RCE=$PATH/Cosmic
#export RCE=$PATH/Cosmic_dataset2
#export RCE_MASKED=$PATH/Cosmic_dataset2-mask
#export RCE_PROCC=$PATH/Cosmic_dataset2-process
#export DUT_PROCC=$PATH/DUT-process

#export DUT=$PATH/run_804_tj_W3R15_50um_6V
export DUT=$PATH/dut_run804
export RCE=$PATH/cosmic_000804_000000
export RCE_MASKED=${RCE}-mask
export RCE_PROCC=${RCE}-process
export DUT_PROCC=${DUT}-process

#export RCE=$PATH/Cosmic
#export RCE=$PATH/Cosmic
#export RCE_MASKED=$PATH/Cosmic-mask
#export RCE_PROCC=$PATH/Cosmic-process

#new
#export RCE=$PATH/cosmic_000610
#export RCE_MASKED=$PATH/cosmic_000610
#export RCE_PROCC=$PATH/cosmic_000610-process

echo "TowerJazz Efficiency Analysis --- 0 TILT"

echo "-------------------Apply mask"
#./Judith -c applyMask -i $RCE.root -o ${RCE_MASKED}_sync.root  -r configs/reforig_TowerJazz.cfg -t configs/globalorig_TowerJazz.cfg #-n 50000

echo "-------------------syncronization"
#./Judith -c synchronizeRMS -i ${RCE_MASKED}.root -o ${RCE_MASKED}_sync.root -I $DUT.root -O ${DUT}_sync.root -r configs/reforig_TowerJazz.cfg -d configs/dutTowerJazz.cfg -t configs/globalorig_TowerJazz.cfg -s 0 -n 30000
./Judith -c synchronize -i ${RCE_MASKED}.root -o ${RCE_MASKED}_sync.root -I $DUT.root -O ${DUT}_sync.root -r configs/reforig_TowerJazz.cfg -d configs/dutTowerJazz.cfg -t  configs/globalorig_TowerJazz.cfg -s 0 #-n 300000

echo "-------------------CoarseAlign telescope"
#./Judith -c coarseAlign -i $ALIG -r configs/reforig_TowerJazz.cfg -t configs/globalorig_TowerJazz.cfg  -n 50000
echo "-------------------FineAlign telescope"
#./Judith -c fineAlign -i $ALIG -r configs/reforig_TowerJazz.cfg -t configs/globalorig_TowerJazz.cfg -n 50000 -R ${ALIG}-alignment-result.root
#
## change to the sync names
export RCE_MASKED=${RCE_MASKED}_sync
export DUT=${DUT}_sync
echo "-------------------CoarseAlign dut"
#./Judith -c coarseAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -r configs/reforig_TowerJazz.cfg -t configs/globalorig.cfg -d configs/dutTowerJazz.cfg
#echo "-------------------FineAlign dut"
#./Judith -c fineAlignDUT -i ${RCE_MASKED}.root -I ${DUT}.root -t configs/globalorig.cfg -r configs/reforig_TowerJazz.cfg -d configs/dutTowerJazz.cfg -R DUT-alignment-result.root -n 50000


echo "-------------------process telescope"
#./Judith -c process -i ${RCE_MASKED}.root -o ${RCE_PROCC}.root -r configs/reforig_TowerJazz.cfg -t configs/globalorig_TowerJazz.cfg -R ${RCE}-proccess-result.root #-n 50000
echo "-------------------process dut"
#./Judith -c process -i ${DUT}.root -o ${DUT_PROCC}.root -r configs/dutTowerJazz.cfg -t configs/globalorig.cfg -R ${DUT}-procress-result.root
#-n 50000
echo "-------------------Analysis telescope"
#./Judith -c analysis -i ${RCE_PROCC}.root -r configs/reforig_TowerJazz.cfg -t configs/globalorig.cfg -R ${RCE}-analysis-result.root -n 50000
echo "-------------------Analysis dut"
#./Judith -c analysisDUT -i ${RCE_PROCC}.root -I ${DUT_PROCC}.root -r configs/reforig_TowerJazz.cfg -d configs/dutTowerJazz.cfg -t configs/globalorig_TowerJazz.cfg -R ${DUT}-analysis-result.root -n 10000

