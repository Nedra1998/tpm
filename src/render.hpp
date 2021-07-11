#ifndef RENDER_HPP_O0ZIUJK8
#define RENDER_HPP_O0ZIUJK8

#include <cstdint>
#include <filesystem>
#include <vector>

#include <CL/sycl.hpp>

#include "exit_code.hpp"
#include "scene.hpp"

namespace tpm {

inline std::uint32_t taus_step(std::uint32_t z, std::int32_t s1,
                               std::int32_t s2, std::int32_t s3,
                               std::uint32_t m) {
  std::uint32_t b = (((z << s1) ^ z) >> s2);
  return (((z & m) << s3) ^ b);
}
inline std::uint32_t lcg_step(std::uint32_t z, std::uint32_t a,
                              std::uint32_t c) {
  return (a * z + c);
}
inline float random(cl::sycl::uint4 &state) {
  state[0] = taus_step(state[0], 13, 19, 12, 4294967294);
  state[1] = taus_step(state[1], 2, 25, 4, 4294967288);
  state[2] = taus_step(state[2], 3, 11, 17, 4294967280);
  state[3] = lcg_step(state[3], 1664525, 1013904223);

  return 2.3283064365387e-10f *
         static_cast<float>(state[0] ^ state[1] ^ state[2] ^ state[3]);
}

struct Image {
  Image(const std::uint32_t &width, const std::uint32_t &height,
        const std::uint32_t &tile_size = 32)
      : size(width, height, tile_size), buffer(width * height) {}
  Image(const cl::sycl::uint3 &size) : size(size), buffer() {}
  Image(const cl::sycl::uint3 &size, std::vector<cl::sycl::float3> &buffer)
      : size(size), buffer(buffer) {}

  inline cl::sycl::uint4 tile(const cl::sycl::uint2 &id) const {
    std::uint32_t x = id[0] * size[2], y = id[1] * size[2];
    return cl::sycl::uint4(x, y, std::min(x + size[2], size[0]),
                           std::min(y + size[2], size[1]));
  }

  static inline cl::sycl::uint4 tile(const cl::sycl::uint3 &size,
                                     const cl::sycl::uint2 &id) {
    std::uint32_t x = id[0] * size[2], y = id[1] * size[2];
    return cl::sycl::uint4(x, y, std::min(x + size[2], size[0]),
                           std::min(y + size[2], size[1]));
  }

  inline std::uint32_t idx(const cl::sycl::uint2 &coord) const {
    return coord[1] * size[0] + coord[0];
  }

  static inline std::uint32_t idx(const cl::sycl::uint3 &size,
                                  const cl::sycl::uint2 &coord) {
    return coord[1] * size[0] + coord[0];
  }

  inline cl::sycl::uint2 tile_size() const {
    return cl::sycl::uint2(size[0] / size[2] + (size[0] % size[2] ? 1 : 0),
                           size[1] / size[2] + (size[1] % size[2] ? 1 : 0));
  }

  cl::sycl::uint3 size;
  std::vector<cl::sycl::float3> buffer;
};

float eval_sdf(
    const cl::sycl::float3 &p, const std::size_t &id,
    const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdf,
    std::size_t &mat);
cl::sycl::float3
ray_march(const cl::sycl::float3 &p, const cl::sycl::float3 &d,
          const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdfs,
          const cl::sycl::accessor<Mat, 1, cl::sycl::access::mode::read> &mats);
cl::sycl::float3 render_pixel(
    const cl::sycl::uint4 &pixel, cl::sycl::uint4 &seed,
    const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdfs,
    const cl::sycl::accessor<Mat, 1, cl::sycl::access::mode::read> &mats);
ExitCode render_frame(const TpmSpec &spec);

ExitCode write(const std::filesystem::path &path, const Image &img);
} // namespace tpm

#endif /* end of include guard: RENDER_HPP_O0ZIUJK8 */
