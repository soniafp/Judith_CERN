#!/bin/sh


# ============================== Alignment run ================================
#not synchronised!!
#rm alignment-ref.dat
#REF="../DataOct2014/ELSC/2014-10-30/cosmic_000006/cosmic_000006_000000-judith" #.root
#./Judith -c coarseAlign -i $REF.root -t configs/global.cfg -r configs/ref.cfg -n 1000000
#./Judith -c fineAlign -i $REF.root -t configs/global.cfg -r configs/ref.cfg -n 10000
#./Judith -c process -i $REF.root -o $REF-p.root -t configs/global.cfg -r configs/ref.cfg -R $REF-results.root -n 100000



# ============================== Analysis run =================================

REF="../DataOct2014/ELSC/2014-10-30/cosmic_000016/cosmic_000016_000000-judith" #.root
DUT="../DataOct2014/ELSC/2014-10-30/conv/RUN000016/out" #.root

./Judith -c synchronize -i $REF.root -o $REF-s.root -I $DUT.root -O $DUT-s.root  -t configs/global.cfg -r configs/ref.cfg -d configs/dutDiamond.cfg
#./Judith -c coarseAlign -i $REF-s.root -t configs/global.cfg -r configs/ref.cfg
#./Judith -c fineAlign -i $REF-s.root -t configs/global.cfg -r configs/ref.cfg -n 10000
#./Judith -c coarseAlignDUT -i $REF-s.root -I $DUT-s.root -t configs/global.cfg -r configs/ref.cfg -d configs/dutDiamond.cfg
#./Judith -c fineAlignDUT -i $REF-s.root -I $DUT-s.root -t configs/global.cfg -r configs/ref.cfg -d configs/dutDiamond.cfg

./Judith -c process -i $REF-s.root -o $REF-sp.root -t configs/global.cfg -r configs/ref.cfg -R $REF-results.root
./Judith -c process -i $DUT-s.root -o $DUT-sp.root -t configs/global.cfg -r configs/dutDiamond.cfg -R $DUT-results.root
./Judith -c analysisDUT -i $REF-sp.root -I $DUT-sp.root -t configs/global.cfg -r configs/ref.cfg -d configs/dutDiamond.cfg -R $DUT-final-analysis.root
