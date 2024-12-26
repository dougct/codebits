bench: RotateBench.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -lbenchmark -o rotate_bench RotateBench.cpp

test: RotateTests.cpp
	clang++ -O2 -std=c++20 -Wall -Wextra -lgtest -lpthread -o rotate_tests RotateTests.cpp

clean:
	rm -f rotate_bench rotate_tests
