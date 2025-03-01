CXX = clang++
CXXFLAGS = -Wall -Wextra -pthread -std=c++20
DEFINES = -DNTHREADS=40 -DELEMS_PER_THREAD=10000000

all: mutex_test

mutex_test: main.cpp mutex.h
	$(CXX) $(CXXFLAGS) $(DEFINES) main.cpp -o mutex_test

clean:
	rm -f mutex_test
