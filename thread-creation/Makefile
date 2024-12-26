bench: ThreadCreationBench.cpp
	c++ -O2 -std=c++20 -Wall -lpthread -lbenchmark -o thread_creation_bench ThreadCreationBench.cpp

test: ThreadCreationTests.cpp
	c++ -O2 -std=c++20 -Wall -lpthread -lgtest -lgtest_main -o thread_creation_tests ThreadCreationTests.cpp

clean:
	rm -f thread_creation_bench thread_creation_tests
