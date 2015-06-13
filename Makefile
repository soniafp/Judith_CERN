CC := g++
CFLAGS := `root-config --cflags` -g -O3 -Wall
LIB := -Llib -ljudstorage -ljudmechanics -ljudproc -ljudana -ljudloop `root-config --ldflags --glibs` -O2
INC := -Iinclude

# Run these commands before entering targets
$(shell mkdir -p build)
$(shell mkdir -p lib)
$(shell mkdir -p bin)

### Executable ###

bin/judith: build/judith.o build/options.o lib/libjudstorage.a lib/libjudmechanics.a lib/libjudproc.a lib/libjudana.a lib/libjudloop.a
	$(CC) build/options.o build/judith.o $(LIB) -o bin/judith

build/judith.o: src/judith.cxx
	$(CC) $(CFLAGS) $(INC) -c src/judith.cxx -o build/judith.o

build/options.o: src/options.cxx include/options.h
	$(CC) $(CFLAGS) $(INC) -c src/options.cxx -o build/options.o

# Don't see the need for a separate library for utils which are relativly small
# so just link into those libraries that need it
build/utils.o: src/utils.cxx include/utils.h
	$(CC) $(CFLAGS) $(INC) -c src/utils.cxx -o build/utils.o

### Storage library ###

lib/libjudstorage.a: build/hit.o build/cluster.o build/plane.o build/track.o build/event.o build/storageio.o build/storagei.o build/storageo.o
	ar ru lib/libjudstorage.a build/hit.o build/cluster.o build/plane.o build/track.o build/event.o build/storageio.o build/storagei.o build/storageo.o

build/hit.o: src/storage/hit.cxx include/storage/hit.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/hit.cxx -o build/hit.o

build/cluster.o: src/storage/cluster.cxx include/storage/cluster.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/cluster.cxx -o build/cluster.o

build/plane.o: src/storage/plane.cxx include/storage/plane.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/plane.cxx -o build/plane.o

build/track.o: src/storage/track.cxx include/storage/track.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/track.cxx -o build/track.o

build/event.o: src/storage/event.cxx include/storage/event.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/event.cxx -o build/event.o

build/storageio.o: src/storage/storageio.cxx include/storage/storageio.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/storageio.cxx -o build/storageio.o

build/storagei.o: src/storage/storagei.cxx include/storage/storagei.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/storagei.cxx -o build/storagei.o

build/storageo.o: src/storage/storageo.cxx include/storage/storageo.h
	$(CC) $(CFLAGS) $(INC) -c src/storage/storageo.cxx -o build/storageo.o

### Mechanics library ###

lib/libjudmechanics.a: build/alignment.o build/sensor.o build/device.o build/mechparsers.o
	ar ru lib/libjudmechanics.a build/alignment.o build/sensor.o build/device.o build/mechparsers.o

build/alignment.o: src/mechanics/alignment.cxx include/mechanics/alignment.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/alignment.cxx -o build/alignment.o

build/sensor.o: src/mechanics/sensor.cxx include/mechanics/sensor.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/sensor.cxx -o build/sensor.o

build/device.o: src/mechanics/device.cxx include/mechanics/device.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/device.cxx -o build/device.o

build/mechparsers.o: src/mechanics/mechparsers.cxx include/mechanics/mechparsers.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/mechparsers.cxx -o build/mechparsers.o

### Processors library ###

lib/libjudproc.a: build/utils.o build/processor.o build/procclustering.o build/procaligning.o build/proctracking.o
	ar ru lib/libjudproc.a build/utils.o build/processor.o build/procclustering.o build/procaligning.o build/proctracking.o

build/processor.o: src/processors/processor.cxx include/processors/processor.h
	$(CC) $(CFLAGS) $(INC) -c src/processors/processor.cxx -o build/processor.o

build/procclustering.o: src/processors/clustering.cxx include/processors/clustering.h
	$(CC) $(CFLAGS) $(INC) -c src/processors/clustering.cxx -o build/procclustering.o

build/procaligning.o: src/processors/aligning.cxx include/processors/aligning.h
	$(CC) $(CFLAGS) $(INC) -c src/processors/aligning.cxx -o build/procaligning.o

build/proctracking.o: src/processors/tracking.cxx include/processors/tracking.h
	$(CC) $(CFLAGS) $(INC) -c src/processors/tracking.cxx -o build/proctracking.o
	
### Analyzers library ###

lib/libjudana.a: build/utils.o build/analyzer.o build/anacorrelations.o build/anaclusterresiduals.o
	ar ru lib/libjudana.a build/utils.o build/analyzer.o build/anacorrelations.o build/anaclusterresiduals.o

build/analyzer.o: src/analyzers/analyzer.cxx include/analyzers/analyzer.h
	$(CC) $(CFLAGS) $(INC) -c src/analyzers/analyzer.cxx -o build/analyzer.o

build/anacorrelations.o: src/analyzers/correlations.cxx include/analyzers/correlations.h
	$(CC) $(CFLAGS) $(INC) -c src/analyzers/correlations.cxx -o build/anacorrelations.o

build/anaclusterresiduals.o: src/analyzers/clusterresiduals.cxx include/analyzers/clusterresiduals.h
	$(CC) $(CFLAGS) $(INC) -c src/analyzers/clusterresiduals.cxx -o build/anaclusterresiduals.o

### Loopers library ###

lib/libjudloop.a: build/utils.o build/looper.o build/loopprocess.o build/loopaligncorr.o
	ar ru lib/libjudloop.a build/utils.o build/looper.o build/loopprocess.o build/loopaligncorr.o

build/looper.o: src/loopers/looper.cxx include/loopers/looper.h
	$(CC) $(CFLAGS) $(INC) -c src/loopers/looper.cxx -o build/looper.o

build/loopprocess.o: src/loopers/loopprocess.cxx include/loopers/loopprocess.h
	$(CC) $(CFLAGS) $(INC) -c src/loopers/loopprocess.cxx -o build/loopprocess.o

build/loopaligncorr.o: src/loopers/loopaligncorr.cxx include/loopers/loopaligncorr.h
	$(CC) $(CFLAGS) $(INC) -c src/loopers/loopaligncorr.cxx -o build/loopaligncorr.o

clean:
	rm -rf build/ lib/* bin/*

.PHONY: clean
