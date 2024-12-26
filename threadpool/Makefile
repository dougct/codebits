all: ThreadPool.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -o threadpool ThreadPool.cpp

test: ThreadPoolTests.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -o threadpool_tests ThreadPoolTests.cpp

bench: ThreadPoolBench.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -lbenchmark -o threadpool_bench ThreadPoolBench.cpp

clean:
	rm -f threadpool threadpool_tests threadpool_bench
