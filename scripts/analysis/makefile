#http://stackoverflow.com/questions/27551363/parallel-in-place-sort-for-numpy-arrays/28663374#28663374
#Build a wrapper for parallel sorting numpy arrays
NUMPY_INC=-I/project/CI-WATER/nfrazie1/local/lib/python2.7/site-packages/numpy/core/include
LIB=-L/project/CI-WATER/nfrazie1/local/lib

all: parallelSort

parallelSort: parallelSort.o
	g++  -g -march=native -O3 -shared  -o parallelSort.so parallelSort.o $(LIB) `python-config --libs` -lgomp 

parallelSort.o: parallelSort.cpp
	g++  -g -march=native -O3 -fpic -c    parallelSort.cpp -o parallelSort.o -fopenmp $(NUMPY_INC) `python-config --includes`

parallelSort.cpp:
	cython --cplus parallelSort.pyx

clean:
	rm -f parallelSort.cpp *.o *.so
