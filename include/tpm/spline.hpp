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

  const std::pair<U, T> &first() const { return keyframes.first(); }
  const std::pair<U, T> &last() const { return keyframes.last(); }
  typename std::vector<std::pair<U, T>>::iterator begin() {
    return keyframes.begin();
  }
  typename std::vector<std::pair<U, T>>::const_iterator begin() const {
    return keyframes.begin();
  }
  typename std::vector<std::pair<U, T>>::iterator end() {
    return keyframes.end();
  }
  typename std::vector<std::pair<U, T>>::const_iterator end() const {
    return keyframes.end();
  }

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
    if (p2 == keyframes.end()) {
      return keyframes.back().second;
    } else if (p2 == keyframes.begin() || t > p2->first) {
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

  std::vector<std::pair<U, T>> keyframes;
};
} // namespace tpm

#ifndef FMT_RANGES_H_
template <template <typename, typename> class T, typename U, typename V,
          typename Char>
struct fmt::formatter<
    T<U, V>, Char,
    typename std::enable_if<std::is_same<tpm::Spline<U, V>, T<U, V>>::value,
                            void>::type> {

  std::basic_string<Char> spec;
  Char prefix = '{', postfix = '}';

  template <typename OutputIt> OutputIt write_delimiter(OutputIt out) {
    *out++ = ',';
    *out++ = ' ';
    return out;
  }

  template <typename OutputIt> OutputIt copy(Char ch, OutputIt out) {
    *out++ = ch;
    return out;
  }

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) -> decltype(ctx.begin()) {
    auto it = ctx.begin();
    auto end = ctx.end();
    while (it != end && (*it != '}')) {
      spec += *it++;
    }
    return it;
  }

  template <typename FormatContext>
  typename FormatContext::iterator format(const T<U, V> &values,
                                          FormatContext &ctx) {
    auto out = copy(prefix, ctx.out());
    auto it = values.keyframes.begin();
    auto end = values.keyframes.end();
    size_t i = 0;

    for (; it != end; ++it) {
      if (i > 0)
        out = write_delimiter(out);
      out = format_to(out, "({0:" + spec + "}: {1:" + spec + "})", it->first,
                      it->second);
      ++i;
    }
    return copy(postfix, copy(postfix, out));
  }
};
#endif

#endif /* end of include guard: TPM_SPLINE_HPP_ */
