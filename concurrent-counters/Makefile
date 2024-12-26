
test:
ifeq ($(version),exact)
		clang++ -std=c++20 -Wall -Wextra -lgtest ExactCounterTests.cpp -o exact_counter_tests
		./exact_counter_tests
else
		clang++ -std=c++20 -Wall -Wextra -lgtest ApproxCounterTests.cpp -o approx_counter_tests
		./approx_counter_tests
endif

bench:
		clang++ -std=c++20 -Wall -Wextra -O3 ConcurrentCountersBench.cpp -lbenchmark -lgtest -o concurrent_counters_bench
		./concurrent_counters_bench --benchmark_report_aggregates_only=true

clean:
		rm -rf exact_counter_tests approx_counter_tests concurrent_counters_bench
