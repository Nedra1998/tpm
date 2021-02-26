#ifndef TPM_GLM_OPS_HPP_
#define TPM_GLM_OPS_HPP_

#include <compare>
#include <iostream>
#include <fmt/format.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vector_relational.hpp>

namespace glm {
template <typename vecType>
inline auto operator<=>(const vecType &lhs, const vecType &rhs) {
  return glm::all(glm::lessThan(lhs, rhs))   ? std::weak_ordering::less
         : glm::all(glm::lessThan(rhs, lhs)) ? std::weak_ordering::greater
                                             : std::weak_ordering::equivalent;
}
inline std::ostream& operator<<(std::ostream& out, const glm::uvec2& rhs) {
  return out << fmt::format("[{}, {}]", rhs.x, rhs.y);
}
inline std::ostream& operator<<(std::ostream& out, const glm::vec2& rhs) {
  return out << fmt::format("[{}, {}]", rhs.x, rhs.y);
}
inline std::ostream& operator<<(std::ostream& out, const glm::vec3& rhs) {
  return out << fmt::format("[{}, {}, {}]", rhs.x, rhs.y, rhs.z);
}
} // namespace glm

#endif /* end of include guard: TPM_GLM_OPS_HPP_ */
