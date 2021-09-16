#include <algorithm>
#include <chrono>
#include <list>
#include <type_traits>
#include <vector>

// Implementations of rotate for forward iterators

template<typename ForwardIterator>
void
rotate_forward_void(ForwardIterator first, ForwardIterator middle, ForwardIterator last) {
    for (ForwardIterator m = middle; ;) {
        std::iter_swap(first, m);
        if (++first == middle) {
            if (++m == last) {
                break;
            } else {
                middle = m;
            }
        } else if (++m == last) {
            m = middle;
        }
    }
}

template<typename ForwardIterator>
ForwardIterator
rotate_forward(ForwardIterator first, ForwardIterator middle, ForwardIterator last) {
    if (first == middle) {
        return last;
    } else if (last == middle) {
        return first;
    }

    ForwardIterator m = middle;
    do { // rotate the first cycle
        std::iter_swap(first, m);
        ++first;
        ++m;
        if (first == middle) {
            middle = m;
        }
    } while (m != last);

    ForwardIterator ret = first;
    m = middle;
    while (m != last) { // rotate subsequent cycles
        std::iter_swap(first, m);
        ++first;
        ++m;
        if (first == middle) {
            middle = m;
        } else if (m == last) {
            m = middle;
        }
    }

    return ret;
}


template<typename ForwardIterator>
void
rotate_cycle(ForwardIterator &first, ForwardIterator &middle, ForwardIterator last) {
    ForwardIterator m = middle;
    do {
        std::iter_swap(first, m);
        ++first;
        ++m;
        if (first == middle) {
            middle = m;
        }
    } while (m != last);
}


template<typename ForwardIterator>
ForwardIterator
rotate_forward_cycles(ForwardIterator first, ForwardIterator middle, ForwardIterator last) {
    if (first == middle) {
        return last;
    } else if (last == middle) {
        return first;
    }

    rotate_cycle(first, middle, last);
    ForwardIterator ret = first;

    while (middle != last) {
        rotate_cycle(first, middle, last);
    }

    return ret;
}


// Implementations of rotate for bidirectional iterators

template<typename BidirectionalIterator>
void
rotate_bidirectional_void(BidirectionalIterator first, BidirectionalIterator middle, BidirectionalIterator last) {
    std::reverse(first,  middle);
    std::reverse(middle, last);
    std::reverse(first, last);
}


template<typename BidirectionalIterator>
BidirectionalIterator
rotate_bidirectional(BidirectionalIterator first, BidirectionalIterator middle, BidirectionalIterator last) {
    if (first == middle) {
        return last;
    } else if (last == middle) {
        return first;
    }

    std::reverse(first,  middle);
    std::reverse(middle, last);

    while (first != middle && middle != last) {
        std::iter_swap(first, --last);
        ++first;
    }

    if (first == middle) {
        std::reverse(middle, last);
        return last;
    } else {
        std::reverse(first,  middle);
        return first;
    }
}


// Implementations of rotate for random access iterators

template<typename Integral>
Integral
algo_gcd(Integral x, Integral y) {
    do {
        Integral t = x % y;
        x = y;
        y = t;
    } while (y);
    return x;
}

template<typename RandomAccessIterator>
RandomAccessIterator
rotate_gcd(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last) {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type difference_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;

    const difference_type k = middle - first;
    const difference_type n = last - middle;
    if (k == n) {
        std::swap_ranges(first, middle, middle);
        return middle;
    }

    const difference_type g = algo_gcd(k, n);
    for (RandomAccessIterator p = first + g; p != first;) {
        value_type t(std::move(*--p));
        RandomAccessIterator i = p;
        RandomAccessIterator j = i + k;
        
        do {
            *i = std::move(*j);
            i = j;
            const difference_type d = last - j;
            if (k < d)
                j += k;
            else
                j = first + (k - d);
        } while (j != p);
        *i = std::move(t);
    }
    return first + n;
}


template <class RandomAccessIterator, class Distance, class T>
void rotate_cycle(RandomAccessIterator first,
                    RandomAccessIterator last,
                    RandomAccessIterator initial,
                    Distance shift,
                    T) {
  T value = *initial;
  RandomAccessIterator i = initial;
  RandomAccessIterator j = i + shift;
  while (j != initial) {
    *i = *j;
    i = j;
    if (last - j > shift)
      j += shift;
    else
      j = first + (shift - (last - j));
  }
  *i = value;
}


template<typename RandomAccessIterator>
RandomAccessIterator
rotate_gcd_stepanov(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last) {
    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type difference_type;
    typedef typename std::iterator_traits<RandomAccessIterator>::value_type value_type;

    const difference_type n = last - first;
    const difference_type k = middle - first;
    
    if (k == n - k) {
        std::swap_ranges(first, middle, middle);
        return middle;
    }

    difference_type ncycles = algo_gcd(n, k);
    while (ncycles--) {
        rotate_cycle(first, last, first + ncycles, k, value_type(*first));
    }
    return first + (n - k);
}


template<typename RandomAccessIterator>
RandomAccessIterator
rotate_random_access(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last) {
    // FIXME: This is different from libstdcxx's implementation
    if (first == middle) {
        return first;
    } else if (last == middle) {
        return middle;
    }

    typedef typename std::iterator_traits<RandomAccessIterator>::difference_type Distance;
    //typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;

    Distance n = last - first;
    Distance k = middle - first;

    if (k == n - k) { // a == b
        std::swap_ranges(first, middle, middle);
        return middle;
    }

    RandomAccessIterator p = first;
    RandomAccessIterator ret = first + (last - middle);

    for (;;) {
        if (k < n - k) { // a < b
            RandomAccessIterator q = p + k;
            for (Distance i = 0; i < n - k; ++i) {
                std::iter_swap(p, q);
                ++p;
                ++q;
            }
        
            n %= k;
            if (n == 0) {
                return ret;
            }
    
            std::swap(n, k);
            k = n - k;
        } else { // a > b
            k = n - k;
            RandomAccessIterator q = p + n;
            p = q - k;
            for (Distance i = 0; i < n - k; ++i) {
                --p;
                --q;
                std::iter_swap(p, q);
            }
            n %= k;
            if (n == 0) {
                return ret;
            }
    
            std::swap(n, k);
        }
    }
}

