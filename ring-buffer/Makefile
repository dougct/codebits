
test:
ifeq ($(version),single)
		clang++ -std=c++20 -Wall -Wextra -lgtest SingleThreadedRingBufferTest.cpp -o single_threaded_ring_buffer_test
		./single_threaded_ring_buffer_test
else
		clang++ -std=c++20 -Wall -Wextra -lgtest RingBufferTest.cpp -o ring_buffer_test
		./ring_buffer_test
endif

bench:
		clang++ -std=c++20 -Wall -Wextra -O3 RingBufferBench.cpp -lbenchmark -lgtest -o ring_buffer_bench
		./ring_buffer_bench --benchmark_report_aggregates_only=true

clean:
		rm -rf single_threaded_ring_buffer_test ring_buffer_test ring_buffer_bench
