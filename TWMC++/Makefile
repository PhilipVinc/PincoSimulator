CC=g++
CFLAGS=-g -I. -I/usr/local/include -I/usr/local/include/eigen3 -pthread -std=c++0x -D_GLIBCXX_USE_NANOSLEEP  -O3 -static-libstdc++  -stdlib=libc++
LINKFLAGS = -L/usr/local/lib -lfftw3 -lm
DEPS = IniPrefReader.hpp TWMC_evolve.hpp WorkerThread.hpp EigenUtils.hpp

UNAME_S := $(shell uname -s)
ifeq ( $(UNAME_S), 'Darwin')
	CFLAGS+=' -stdlib=libc++'
endif

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
	
twmc: TWMC_evolve.o EigenUtils.o IniPrefReader.o WorkerThread.o main.o
	$(CC) -v  -L/usr/local/lib $(CFLAGS)  $^ -o twmc  -I. $(LINKFLAGS)

clean:
	rm -f *.o
	rm -f twmc
	
