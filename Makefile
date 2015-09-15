CC = g++
CFLAGS = `root-config --cflags` -O2 -Wall
LFLAGS = `root-config --ldflags --glibs` -lMathCore -O1
OBJPATH = obj
SRCPATH = src
EXECUTABLE = Judith
OBJECTS = $(OBJPATH)/configparser.o $(OBJPATH)/inputargs.o $(OBJPATH)/main.o $(OBJPATH)/clusterinfo.o $(OBJPATH)/configanalyzers.o $(OBJPATH)/correlation.o $(OBJPATH)/depiction.o $(OBJPATH)/dualanalyzer.o $(OBJPATH)/dutcorrelation.o $(OBJPATH)/dutdepiction.o $(OBJPATH)/dutresiduals.o $(OBJPATH)/efficiency.o $(OBJPATH)/eventinfo.o $(OBJPATH)/exampledualanalyzer.o $(OBJPATH)/examplesingleanalyzer.o $(OBJPATH)/hitinfo.o $(OBJPATH)/matching.o $(OBJPATH)/occupancy.o $(OBJPATH)/residuals.o $(OBJPATH)/singleanalyzer.o $(OBJPATH)/syncfluctuation.o $(OBJPATH)/trackinfo.o $(OBJPATH)/kartelconvert.o $(OBJPATH)/analysis.o $(OBJPATH)/analysisdut.o $(OBJPATH)/applymask.o $(OBJPATH)/chi2align.o $(OBJPATH)/coarsealign.o $(OBJPATH)/coarsealigndut.o $(OBJPATH)/configloopers.o $(OBJPATH)/examplelooper.o $(OBJPATH)/finealign.o $(OBJPATH)/finealigndut.o $(OBJPATH)/looper.o $(OBJPATH)/noisescan.o $(OBJPATH)/processevents.o $(OBJPATH)/synchronize.o $(OBJPATH)/synchronizerms.o $(OBJPATH)/alignment.o $(OBJPATH)/configmechanics.o $(OBJPATH)/device.o $(OBJPATH)/noisemask.o $(OBJPATH)/sensor.o $(OBJPATH)/clustermaker.o $(OBJPATH)/configprocessors.o $(OBJPATH)/eventdepictor.o $(OBJPATH)/largesynchronizer.o $(OBJPATH)/processors.o $(OBJPATH)/synchronizer.o $(OBJPATH)/trackmaker.o $(OBJPATH)/trackmatcher.o $(OBJPATH)/cluster.o $(OBJPATH)/event.o $(OBJPATH)/hit.o $(OBJPATH)/plane.o $(OBJPATH)/storageio.o $(OBJPATH)/track.o 
all: Judith

Judith: $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o $(EXECUTABLE)

$(OBJPATH)/configparser.o: $(SRCPATH)/configparser.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/configparser.cpp -o $(OBJPATH)/configparser.o

$(OBJPATH)/inputargs.o: $(SRCPATH)/inputargs.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/inputargs.cpp -o $(OBJPATH)/inputargs.o

$(OBJPATH)/main.o: $(SRCPATH)/main.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/main.cpp -o $(OBJPATH)/main.o

$(OBJPATH)/clusterinfo.o: $(SRCPATH)/analyzers/clusterinfo.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/clusterinfo.cpp -o $(OBJPATH)/clusterinfo.o

$(OBJPATH)/configanalyzers.o: $(SRCPATH)/analyzers/configanalyzers.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/configanalyzers.cpp -o $(OBJPATH)/configanalyzers.o

$(OBJPATH)/correlation.o: $(SRCPATH)/analyzers/correlation.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/correlation.cpp -o $(OBJPATH)/correlation.o

$(OBJPATH)/depiction.o: $(SRCPATH)/analyzers/depiction.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/depiction.cpp -o $(OBJPATH)/depiction.o

$(OBJPATH)/dualanalyzer.o: $(SRCPATH)/analyzers/dualanalyzer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/dualanalyzer.cpp -o $(OBJPATH)/dualanalyzer.o

$(OBJPATH)/dutcorrelation.o: $(SRCPATH)/analyzers/dutcorrelation.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/dutcorrelation.cpp -o $(OBJPATH)/dutcorrelation.o

$(OBJPATH)/dutdepiction.o: $(SRCPATH)/analyzers/dutdepiction.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/dutdepiction.cpp -o $(OBJPATH)/dutdepiction.o

$(OBJPATH)/dutresiduals.o: $(SRCPATH)/analyzers/dutresiduals.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/dutresiduals.cpp -o $(OBJPATH)/dutresiduals.o

$(OBJPATH)/efficiency.o: $(SRCPATH)/analyzers/efficiency.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/efficiency.cpp -o $(OBJPATH)/efficiency.o

$(OBJPATH)/eventinfo.o: $(SRCPATH)/analyzers/eventinfo.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/eventinfo.cpp -o $(OBJPATH)/eventinfo.o

$(OBJPATH)/exampledualanalyzer.o: $(SRCPATH)/analyzers/exampledualanalyzer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/exampledualanalyzer.cpp -o $(OBJPATH)/exampledualanalyzer.o

$(OBJPATH)/examplesingleanalyzer.o: $(SRCPATH)/analyzers/examplesingleanalyzer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/examplesingleanalyzer.cpp -o $(OBJPATH)/examplesingleanalyzer.o

$(OBJPATH)/hitinfo.o: $(SRCPATH)/analyzers/hitinfo.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/hitinfo.cpp -o $(OBJPATH)/hitinfo.o

$(OBJPATH)/matching.o: $(SRCPATH)/analyzers/matching.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/matching.cpp -o $(OBJPATH)/matching.o

$(OBJPATH)/occupancy.o: $(SRCPATH)/analyzers/occupancy.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/occupancy.cpp -o $(OBJPATH)/occupancy.o

$(OBJPATH)/residuals.o: $(SRCPATH)/analyzers/residuals.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/residuals.cpp -o $(OBJPATH)/residuals.o

$(OBJPATH)/singleanalyzer.o: $(SRCPATH)/analyzers/singleanalyzer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/singleanalyzer.cpp -o $(OBJPATH)/singleanalyzer.o

$(OBJPATH)/syncfluctuation.o: $(SRCPATH)/analyzers/syncfluctuation.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/syncfluctuation.cpp -o $(OBJPATH)/syncfluctuation.o

$(OBJPATH)/trackinfo.o: $(SRCPATH)/analyzers/trackinfo.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/analyzers/trackinfo.cpp -o $(OBJPATH)/trackinfo.o

$(OBJPATH)/kartelconvert.o: $(SRCPATH)/converters/kartelconvert.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/converters/kartelconvert.cpp -o $(OBJPATH)/kartelconvert.o

$(OBJPATH)/analysis.o: $(SRCPATH)/loopers/analysis.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/analysis.cpp -o $(OBJPATH)/analysis.o

$(OBJPATH)/analysisdut.o: $(SRCPATH)/loopers/analysisdut.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/analysisdut.cpp -o $(OBJPATH)/analysisdut.o

$(OBJPATH)/applymask.o: $(SRCPATH)/loopers/applymask.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/applymask.cpp -o $(OBJPATH)/applymask.o

$(OBJPATH)/chi2align.o: $(SRCPATH)/loopers/chi2align.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/chi2align.cpp -o $(OBJPATH)/chi2align.o

$(OBJPATH)/coarsealign.o: $(SRCPATH)/loopers/coarsealign.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/coarsealign.cpp -o $(OBJPATH)/coarsealign.o

$(OBJPATH)/coarsealigndut.o: $(SRCPATH)/loopers/coarsealigndut.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/coarsealigndut.cpp -o $(OBJPATH)/coarsealigndut.o

$(OBJPATH)/configloopers.o: $(SRCPATH)/loopers/configloopers.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/configloopers.cpp -o $(OBJPATH)/configloopers.o

$(OBJPATH)/examplelooper.o: $(SRCPATH)/loopers/examplelooper.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/examplelooper.cpp -o $(OBJPATH)/examplelooper.o

$(OBJPATH)/finealign.o: $(SRCPATH)/loopers/finealign.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/finealign.cpp -o $(OBJPATH)/finealign.o

$(OBJPATH)/finealigndut.o: $(SRCPATH)/loopers/finealigndut.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/finealigndut.cpp -o $(OBJPATH)/finealigndut.o

$(OBJPATH)/looper.o: $(SRCPATH)/loopers/looper.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/looper.cpp -o $(OBJPATH)/looper.o

$(OBJPATH)/noisescan.o: $(SRCPATH)/loopers/noisescan.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/noisescan.cpp -o $(OBJPATH)/noisescan.o

$(OBJPATH)/processevents.o: $(SRCPATH)/loopers/processevents.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/processevents.cpp -o $(OBJPATH)/processevents.o

$(OBJPATH)/synchronize.o: $(SRCPATH)/loopers/synchronize.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/synchronize.cpp -o $(OBJPATH)/synchronize.o

$(OBJPATH)/synchronizerms.o: $(SRCPATH)/loopers/synchronizerms.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/loopers/synchronizerms.cpp -o $(OBJPATH)/synchronizerms.o

$(OBJPATH)/alignment.o: $(SRCPATH)/mechanics/alignment.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/mechanics/alignment.cpp -o $(OBJPATH)/alignment.o

$(OBJPATH)/configmechanics.o: $(SRCPATH)/mechanics/configmechanics.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/mechanics/configmechanics.cpp -o $(OBJPATH)/configmechanics.o

$(OBJPATH)/device.o: $(SRCPATH)/mechanics/device.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/mechanics/device.cpp -o $(OBJPATH)/device.o

$(OBJPATH)/noisemask.o: $(SRCPATH)/mechanics/noisemask.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/mechanics/noisemask.cpp -o $(OBJPATH)/noisemask.o

$(OBJPATH)/sensor.o: $(SRCPATH)/mechanics/sensor.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/mechanics/sensor.cpp -o $(OBJPATH)/sensor.o

$(OBJPATH)/clustermaker.o: $(SRCPATH)/processors/clustermaker.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/clustermaker.cpp -o $(OBJPATH)/clustermaker.o

$(OBJPATH)/configprocessors.o: $(SRCPATH)/processors/configprocessors.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/configprocessors.cpp -o $(OBJPATH)/configprocessors.o

$(OBJPATH)/eventdepictor.o: $(SRCPATH)/processors/eventdepictor.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/eventdepictor.cpp -o $(OBJPATH)/eventdepictor.o

$(OBJPATH)/largesynchronizer.o: $(SRCPATH)/processors/largesynchronizer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/largesynchronizer.cpp -o $(OBJPATH)/largesynchronizer.o

$(OBJPATH)/processors.o: $(SRCPATH)/processors/processors.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/processors.cpp -o $(OBJPATH)/processors.o

$(OBJPATH)/synchronizer.o: $(SRCPATH)/processors/synchronizer.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/synchronizer.cpp -o $(OBJPATH)/synchronizer.o

$(OBJPATH)/trackmaker.o: $(SRCPATH)/processors/trackmaker.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/trackmaker.cpp -o $(OBJPATH)/trackmaker.o

$(OBJPATH)/trackmatcher.o: $(SRCPATH)/processors/trackmatcher.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/processors/trackmatcher.cpp -o $(OBJPATH)/trackmatcher.o

$(OBJPATH)/cluster.o: $(SRCPATH)/storage/cluster.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/cluster.cpp -o $(OBJPATH)/cluster.o

$(OBJPATH)/event.o: $(SRCPATH)/storage/event.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/event.cpp -o $(OBJPATH)/event.o

$(OBJPATH)/hit.o: $(SRCPATH)/storage/hit.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/hit.cpp -o $(OBJPATH)/hit.o

$(OBJPATH)/plane.o: $(SRCPATH)/storage/plane.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/plane.cpp -o $(OBJPATH)/plane.o

$(OBJPATH)/storageio.o: $(SRCPATH)/storage/storageio.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/storageio.cpp -o $(OBJPATH)/storageio.o

$(OBJPATH)/track.o: $(SRCPATH)/storage/track.cpp
	$(CC) $(CFLAGS) -c $(SRCPATH)/storage/track.cpp -o $(OBJPATH)/track.o

clean:
	 rm $(OBJPATH)/*.o Judith

.PHONY: clean
