CXX = clang++
PORTABLE ?= 1
CXXFLAGS = -Wall -Wextra -pthread -std=c++20 -DPORTABLE=$(PORTABLE)
DEFINES = -DNTHREADS=40 -DELEMS_PER_THREAD=10000000

all: mutex_test

mutex_test: main.cpp futex_wrapper.h portable_mutex.h futex_based_mutex.h
	$(CXX) $(CXXFLAGS) $(DEFINES) main.cpp -o mutex_test

clean:
	rm -f mutex_test
