#define NUM_THREADS 2    /* Assuming some number of threads */
#define THRESHOLD 2     /* Assuming some threshold value */

/* Global variables */
int global_counter = 0;
byte local_counters[NUM_THREADS];
byte num_updates = 0;

proctype buggy_counter_process(byte amount) {
    byte idx;

    num_updates = num_updates + 1;
    idx = num_updates % NUM_THREADS;
    local_counters[idx] = local_counters[idx] + amount;

    if
    :: (num_updates >= THRESHOLD) ->
        num_updates = 0;
        global_counter = global_counter + local_counters[idx];
        local_counters[idx] = 0;
    :: else -> skip
    fi;
}


proctype counter_process(byte amount) {
    byte idx;

    num_updates = num_updates + 1;
    atomic {
        idx = num_updates % NUM_THREADS;
        local_counters[idx] = local_counters[idx] + amount;
    }

    if
    :: (num_updates >= THRESHOLD) ->
        num_updates = 0;
        byte c;
        for (c : 0 .. NUM_THREADS - 1) {
            atomic {
                global_counter = global_counter + local_counters[c];
                local_counters[c] = 0;
            }
        }
    :: else -> skip
    fi;
}

init {
    byte i;
	for (i : 1 .. 2) {
        atomic {
            run buggy_counter_process(1);
            run buggy_counter_process(1);
        }
    }
    (_nr_pr == 1) -> printf("global_counter = %d\n", global_counter)
    assert (global_counter <= 5)
}

/*
spin count.pml
spin -a count.pml
clang -o pan pan.c
./pan
spin -p -t -g count.pml

$ spin -p -t count.pml


int64_t update(int64_t amount) {
    // Round-robin updating local counters
    const uint32_t idx = num_updates_++ % num_threads_;
    local_counters_[idx] += amount;
    if (num_updates_ >= threshold_) {
        global_counter_ += local_counters_[idx];
        local_counters_[idx] = 0;
        num_updates_ = 0;
    }
    return global_counter_;
}
*/
