bench: thread_creation_bench.cpp
	c++ -O2 -std=c++20 -Wall -lpthread -lbenchmark -o thread_creation_bench thread_creation_bench.cpp

test: test_thread_creation.cpp
	c++ -O2 -std=c++20 -Wall -lpthread -lgtest -lgtest_main -o test_thread_creation test_thread_creation.cpp

clean:
	rm -f thread_creation_bench test_thread_creation
