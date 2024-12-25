#include <algorithm>
#include <chrono>
#include <list>
#include <type_traits>
#include <vector>

// Implementations of rotate for forward iterators

template <typename FwdIt>
void rotate_forward_void(FwdIt first, FwdIt middle, FwdIt last) {
  for (FwdIt m = middle;;) {
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

template <typename FwdIt>
FwdIt rotate_forward(FwdIt first, FwdIt middle, FwdIt last) {
  if (first == middle) {
    return last;
  } else if (last == middle) {
    return first;
  }

  FwdIt m = middle;
  do {  // rotate the first cycle
    std::iter_swap(first, m);
    ++first;
    ++m;
    if (first == middle) {
      middle = m;
    }
  } while (m != last);

  FwdIt ret = first;
  m = middle;
  while (m != last) {  // rotate subsequent cycles
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

template <typename FwdIt>
void rotate_cycle(FwdIt& first, FwdIt& middle, FwdIt last) {
  FwdIt m = middle;
  do {
    std::iter_swap(first, m);
    ++first;
    ++m;
    if (first == middle) {
      middle = m;
    }
  } while (m != last);
}

template <typename FwdIt>
FwdIt rotate_forward_cycles(FwdIt first, FwdIt middle, FwdIt last) {
  if (first == middle) {
    return last;
  } else if (last == middle) {
    return first;
  }

  rotate_cycle(first, middle, last);
  FwdIt ret = first;

  while (middle != last) {
    rotate_cycle(first, middle, last);
  }

  return ret;
}

// Implementations of rotate for bidirectional iterators

template <typename BidIt>
void rotate_bidirectional_void(BidIt first, BidIt middle, BidIt last) {
  std::reverse(first, middle);
  std::reverse(middle, last);
  std::reverse(first, last);
}

template <typename BidIt>
BidIt rotate_bidirectional(BidIt first, BidIt middle, BidIt last) {
  if (first == middle) {
    return last;
  } else if (last == middle) {
    return first;
  }

  std::reverse(first, middle);
  std::reverse(middle, last);

  while (first != middle && middle != last) {
    std::iter_swap(first, --last);
    ++first;
  }

  if (first == middle) {
    std::reverse(middle, last);
    return last;
  } else {
    std::reverse(first, middle);
    return first;
  }
}

// Implementations of rotate for random access iterators

template <typename Integral>
Integral algo_gcd(Integral x, Integral y) {
  do {
    Integral t = x % y;
    x = y;
    y = t;
  } while (y);
  return x;
}

template <typename RandAcIt>
RandAcIt rotate_gcd(RandAcIt first, RandAcIt middle, RandAcIt last) {
  using difference_type =
      typename std::iterator_traits<RandAcIt>::difference_type;
  using value_type = typename std::iterator_traits<RandAcIt>::value_type;

  const difference_type n = last - first;
  const difference_type k = middle - first;

  if (k == n - k) {
    std::swap_ranges(first, middle, middle);
    return middle;
  }

  const difference_type ncycles = algo_gcd(k, n);
  for (RandAcIt p = first + ncycles; p != first;) {
    value_type t(std::move(*--p));
    RandAcIt i = p;
    RandAcIt j = i + k;

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

  return first + (n - k);
}

template <class RandAcIt, class Dist, class T>
void rotate_cycle(RandAcIt first,
                  RandAcIt last,
                  RandAcIt initial,
                  Dist shift,
                  T) {
  T value = *initial;
  RandAcIt i = initial;
  RandAcIt j = i + shift;
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

template <typename RandAcIt>
RandAcIt rotate_gcd_stepanov(RandAcIt first, RandAcIt middle, RandAcIt last) {
  using difference_type =
      typename std::iterator_traits<RandAcIt>::difference_type;
  using value_type = typename std::iterator_traits<RandAcIt>::value_type;

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
