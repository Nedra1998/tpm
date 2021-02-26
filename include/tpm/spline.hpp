#ifndef TPM_SPLINE_HPP_
#define TPM_SPLINE_HPP_

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace tpm {
template <typename T, typename U = float> class Spline {
public:
  Spline(const T &v) {
    keyframes.push_back(std::pair<U, T>(static_cast<U>(0.0), v));
  }

  const U &begin() const { return keyframes.first().first; }
  const U &end() const { return keyframes.last().first; }

  void insert(const U &t, const T &v) {
    typename std::vector<std::pair<U, T>>::iterator it = std::upper_bound(
        keyframes.begin(), keyframes.end(), std::pair<U, T>{t, v});
    if (it != keyframes.end() && it->first == t) {
      it->second = v;
    } else {
      keyframes.insert(it, std::pair<U, T>{t, v});
    }
  }

  T operator[](const U &t) const {
    typename std::vector<std::pair<U, T>>::const_iterator p2 = std::lower_bound(
        keyframes.begin(), keyframes.end(), std::pair<U, T>{t, T()});
    if (p2 == keyframes.begin() || p2 == keyframes.end() || t > p2->first) {
      return p2->second;
    } else if (t <= p2->first + static_cast<U>(10.0) *
                                    std::numeric_limits<U>::epsilon() &&
               t >= p2->first - static_cast<U>(10.0) *
                                    std::numeric_limits<U>::epsilon()) {
      return p2->second;
    }
    typename std::vector<std::pair<U, T>>::const_iterator p1 = p2 - 1;
    typename std::vector<std::pair<U, T>>::const_iterator p0 = p1 - 1;
    typename std::vector<std::pair<U, T>>::const_iterator p3 = p2 + 1;

    T m1 = (p1 == keyframes.begin())
               ? (p2->second - p1->second) / (p2->first - p1->first)
               : static_cast<U>(0.5) *
                     ((p2->second - p1->second) / (p2->first - p1->first) +
                      (p1->second - p0->second) / (p1->first - p0->first));
    T m2 = (p3 == keyframes.end())
               ? (p2->second - p1->second) / (p2->first - p1->first)
               : static_cast<U>(0.5) *
                     ((p3->second - p2->second) / (p3->first - p2->first) +
                      (p2->second - p1->second) / (p2->first - p1->first));

    U u = (t - p1->first) / (p2->first - p1->first);
    return (1 + 2 * u) * (1 - u) * (1 - u) * p1->second +
           u * (1 - u) * (1 - u) * (p2->first - p1->first) * m1 +
           u * u * (3 - 2 * u) * p2->second +
           u * u * (u - 1) * (p2->first - p1->first) * m2;
  }

  friend std::ostream &operator<<(std::ostream &out,
                                  const Spline<T, U> &spline);

private:
  std::vector<std::pair<U, T>> keyframes;
};

// template <typename T, typename U>
// inline std::ostream &operator<<(std::ostream &out, const Spline<T, U> &spline) {
//   return out << fmt::format("{}", spline.keyframes);
// }
} // namespace tpm

#endif /* end of include guard: TPM_SPLINE_HPP_ */
