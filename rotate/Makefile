bench: benchmark_rotate.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -lbenchmark -o benchmark_rotate benchmark_rotate.cpp

test: test_rotate.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -o test_rotate test_rotate.cpp

clean:
	rm -f benchmark_rotate test_rotate
