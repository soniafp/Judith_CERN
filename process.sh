#!/bin/sh

REF="ref01-01"
DUT="dut01-01"

#./Judith -c synchronize -i $REF.root -I $DUT.root -o $REF-s.root -O $DUT-s.root -r configs/desy-kartel.cfg -d configs/desy-dut.cfg -t configs/desy-tb.cfg
### Alignment ###
#./Judith -c coarseAlign -i $REF-s.root -r configs/desy-kartel.cfg -t configs/desy-tb.cfg -n 10000
#./Judith -c fineAlign -i $REF-s.root -r configs/desy-kartel.cfg -t configs/desy-tb.cfg -n 10000
#./Judith -c coarseAlignDUT -i $REF-s.root -I $DUT-s.root -r configs/desy-kartel.cfg -d configs/desy-dut.cfg -t configs/desy-tb.cfg -n 20000
#./Judith -c fineAlignDUT -i $REF-s.root -I $DUT-s.root -r configs/desy-kartel.cfg -d configs/desy-dut.cfg -t configs/desy-tb.cfg -n 20000
### end ##
./Judith -c process -i $REF-s.root -o $REF-sp.root -r configs/desy-kartel.cfg -t configs/desy-tb.cfg -R $REF-results.root
./Judith -c process -i $DUT-s.root -o $DUT-sp.root -r configs/desy-dut.cfg -t configs/desy-tb.cfg -R $DUT-results.root
./Judith -c analysisDUT -i $REF-sp.root -I $DUT-sp.root -r configs/desy-kartel.cfg -d configs/desy-dut.cfg -t configs/desy-tb.cfg -R $REF-$DUT-analysis.root
