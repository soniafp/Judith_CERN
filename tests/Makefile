CC := g++
CFLAGS := `root-config --cflags` -g -O3 -Wall
LIB := -L../lib -ljudstorage `root-config --ldflags --glibs` -O1
INC := -I../include

$(shell mkdir -p bin/)

bin/test_storage: test_storage.cxx
	$(CC) -o bin/test_storage $(CFLAGS) $(INC) test_storage.cxx $(LIB)

clean:
	rm -rf bin/*

.PHONY: clean
