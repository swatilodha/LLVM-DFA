INC=-I/usr/local/include/
# all: liveness.so available.so reaching.so
all: available.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) $(INC) -g -O0 -fPIC

dataflow.o: dataflow.cpp dataflow.h

available-support.o: available-support.cpp available-support.h

# reaching-support.o: reaching-support.cpp reaching-support.h

# %.so: %.o dataflow.o available-support.o reaching-support.o
%.so: %.o dataflow.o available-support.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

.PHONY: clean all