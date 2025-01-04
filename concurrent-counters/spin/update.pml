#define USE_BUGGY_COUNTER 0 /* 1 for buggy, 0 for correct */

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
        byte c;
        for (c : 0 .. NUM_THREADS - 1) {
            global_counter = global_counter + local_counters[c];
            local_counters[c] = 0;
        }
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
            #ifdef USE_BUGGY_COUNTER
                run buggy_counter_process(1);
                run buggy_counter_process(1);
            #else
                run counter_process(1);
                run counter_process(1);
            #endif
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
*/
