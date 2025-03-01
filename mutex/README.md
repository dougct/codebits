# Implementing a mutex in C++

File `mutex.h` contains two implementations of mutex based on the paper "Futexes are Tricky" by Ulrich Drepper [1].

The main difference between this implementation and the one found in [2] is that I'm actually not explicitly using futexes. I'm using native operations on std atomics to accomplish the same thing.

To run the tests, do:

```
make all
./mutex_test
```

# References

1. [Futexes are Tricky](https://cis.temple.edu/~giorgio/cis307/readings/futex.pdf)
2. [Basics of Futexes](https://eli.thegreenplace.net/2018/basics-of-futexes/)
3. [Fuss, Futexes and Furwocks: Fast Userlevel Locking in Linux](https://www.kernel.org/doc/ols/2002/ols2002-pages-479-495.pdf)
