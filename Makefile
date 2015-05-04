CC := g++
CFLAGS := `root-config --cflags` -g -O3 -Wall
LIB := -Llib -ljudstorage -ljudmechanics `root-config --ldflags --glibs` -O1
INC := -Iinclude

# Run these commands before entering targets
$(shell mkdir -p build)
$(shell mkdir -p lib)
$(shell mkdir -p bin)

### Executable ###

bin/judith: build/judith.o build/options.o lib/libjudstorage.a lib/libjudmechanics.a
	$(CC) build/options.o build/judith.o $(LIB) -o bin/judith

build/judith.o: src/judith.cxx
	$(CC) $(CFLAGS) $(INC) -c src/judith.cxx -o build/judith.o

build/options.o: src/options.cxx include/options.h
	$(CC) $(CFLAGS) $(INC) -c src/options.cxx -o build/options.o

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

lib/libjudmechanics.a: build/alignment.o build/sensor.o build/device.o build/parsedevice.o
	ar ru lib/libjudmechanics.a build/alignment.o build/sensor.o build/device.o build/parsedevice.o

build/alignment.o: src/mechanics/alignment.cxx include/mechanics/alignment.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/alignment.cxx -o build/alignment.o

build/sensor.o: src/mechanics/sensor.cxx include/mechanics/sensor.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/sensor.cxx -o build/sensor.o

build/device.o: src/mechanics/device.cxx include/mechanics/device.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/device.cxx -o build/device.o

build/parsedevice.o: src/mechanics/parsedevice.cxx include/mechanics/parsedevice.h
	$(CC) $(CFLAGS) $(INC) -c src/mechanics/parsedevice.cxx -o build/parsedevice.o

clean:
	rm -rf build/ lib/* bin/*

.PHONY: clean
