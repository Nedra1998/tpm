#ifndef SDF_HPP_OAMZXL8I
#define SDF_HPP_OAMZXL8I

#include <CL/sycl.hpp>

namespace tpm::sdf {
inline float dot2(const cl::sycl::float2 &v) { return cl::sycl::dot(v, v); }
inline float dot2(const cl::sycl::float3 &v) { return cl::sycl::dot(v, v); }
inline float ndot(const cl::sycl::float2 &a, const cl::sycl::float2 &b) {
  return a[0] * b[0] - a[1] * b[1];
}
inline float sphere(const cl::sycl::float3 &p, const float &s) {
  return cl::sycl::length(p) - s;
}
inline float box(const cl::sycl::float3 &p, const cl::sycl::float3 &b) {
  cl::sycl::float3 q = cl::sycl::fabs(p) - b;
  return cl::sycl::length(cl::sycl::max(q, cl::sycl::float3(0.0, 0.0, 0.0))) +
         cl::sycl::min(cl::sycl::max(q[0], cl::sycl::max(q[1], q[2])), 0.0f);
}

inline cl::sycl::float3 op_translate(const cl::sycl::float3 &p,
                                  const cl::sycl::float3 &t) {
  return p - t;
}
inline float op_union(const float &a, const float &b) {
  return cl::sycl::min(a, b);
}
} // namespace tpm::sdf

#endif /* end of include guard: SDF_HPP_OAMZXL8I */
