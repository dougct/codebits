# Implementing a mutex in C++

This repository contains two implementations of mutex, both based on the paper "Futexes are Tricky" by Ulrich Drepper [1].

One of the implementations, called futex-based mutex, uses a futex wrapper which works both on Linux and MacOS. The other implementation, called portable mutex, uses only std::atomic primitives.

To run the tests, do:

```
make all PORTABLE=1
./mutex_test
```

# References

1. [Futexes are Tricky](https://cis.temple.edu/~giorgio/cis307/readings/futex.pdf)
2. [Basics of Futexes](https://eli.thegreenplace.net/2018/basics-of-futexes/)
3. [Fuss, Futexes and Furwocks: Fast Userlevel Locking in Linux](https://www.kernel.org/doc/ols/2002/ols2002-pages-479-495.pdf)
