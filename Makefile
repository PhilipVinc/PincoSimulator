CC = g++
INCLUDES = -I. -I/usr/local/include -I/usr/local/include/eigen3
LIBRARIES = -L/usr/local/lib -lfftw3 -lm -static-libstdc++ 
FLAGS = -g -pthread -std=c++11  -D_GLIBCXX_USE_NANOSLEEP  -O3 -static-libstdc++
#

EXECUTABLE_NAME = twmc

SOURCEDIR = ./TWMC++
BUILD_DIR = ./build

INCLUDES += -I$(SOURCEDIR)
CFLAGS= $(INCLUDES) $(FLAGS)
LINKFLAGS =  $(LIBRARIES) $(FLAGS)

INTERMEDIATE_DIR = $(BUILD_DIR)/tmp
SOURCES = $(wildcard $(SOURCEDIR)/*.cpp)
OBJECTS = $(patsubst $(SOURCEDIR)/%.cpp,$(INTERMEDIATE_DIR)/%.o,$(SOURCES))

# Fix for Mac Xcode stupidty
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CC += -stdlib=libc++
endif
# Fix for old version of Matlab in Linux Cluster
ifeq ($(UNAME_S),Linux)
	CC += -static-libstdc++
	# Fix for th2 install of fftw shitty
	LIBRARIES += -L/usr/lib
endif
# End fix

all: dir $(BUILD_DIR)/$(EXECUTABLE_NAME)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(INTERMEDIATE_DIR)

$(BUILD_DIR)/$(EXECUTABLE_NAME): $(OBJECTS)
	$(CC) $^ $(LINKFLAGS) -o $@

$(OBJECTS): $(INTERMEDIATE_DIR)/%.o : $(SOURCEDIR)/%.cpp
	$(CC) -c $< $(CFLAGS) -o $@ 

clean:
	rm -f $(INTERMEDIATE_DIR)/*o $(BUILD_DIR)/$(EXECUTABLE_NAME)

list:
	echo $(SOURCES); echo $(OBJECTS); echo $(UNAME_S); echo $(CC); echo $(TMP);
