#ifndef SCENE_HPP_RZWYUXDC
#define SCENE_HPP_RZWYUXDC

#include <optional>
#include <string>

#include <CL/sycl.hpp>
#include <pugixml.hpp>

#include "exit_code.hpp"

namespace tpm {

enum SdfType { SPHERE, TRANSLATE, UNION };
enum MatType { NONE, EMISSION, DIFFUSE, GLASS, GLOSSY };

struct Mat {
  Mat(const MatType &type, const cl::sycl::float3 &color,
      const cl::sycl::float2 &args)
      : type(type), color(color), args(args) {}
  Mat(const MatType &type, const cl::sycl::float3 &color, const float &args)
      : type(type), color(color), args(args, 0.0f) {}

  MatType type;
  cl::sycl::float3 color;
  cl::sycl::float2 args;
};

struct Sdf {
  Sdf(const SdfType &type, const cl::sycl::float4 &args)
      : type(type), args(args), mat(std::numeric_limits<std::size_t>::max()),
        a(std::numeric_limits<std::size_t>::max()),
        b(std::numeric_limits<std::size_t>::max()) {}
  Sdf(const SdfType &type, const float &a1 = 0.0f, const float &a2 = 0.0f,
      const float &a3 = 0.0f, const float &a4 = 0.0f)
      : type(type), args(a1, a2, a3, a4),
        mat(std::numeric_limits<std::size_t>::max()),
        a(std::numeric_limits<std::size_t>::max()),
        b(std::numeric_limits<std::size_t>::max()) {}
  Sdf(const SdfType &type, const cl::sycl::float3 &args)
      : type(type), args(args, 0.0f),
        mat(std::numeric_limits<std::size_t>::max()),
        a(std::numeric_limits<std::size_t>::max()),
        b(std::numeric_limits<std::size_t>::max()) {}
  Sdf(const SdfType &type, const cl::sycl::float2 &args)
      : type(type), args(args, 0.0f, 0.0f),
        mat(std::numeric_limits<std::size_t>::max()),
        a(std::numeric_limits<std::size_t>::max()),
        b(std::numeric_limits<std::size_t>::max()) {}
  SdfType type;
  cl::sycl::float4 args;
  std::size_t mat, a, b;
};

struct ImageSpec {
  std::string path = "output.png";
  std::uint32_t width = 1920, height = 1080, tile = 32;
};
struct RendererSpec {
  std::size_t spp = 64;
};
struct TpmSpec {
  ImageSpec image;
  RendererSpec renderer;
  std::vector<Sdf> sdfs;
  std::vector<Mat> mats;
};

cl::sycl::float3 parse_hex(const std::string &hex);
std::size_t parse_mat(const pugi::xml_node &node, TpmSpec &spec);

std::size_t parse_sphere(const pugi::xml_node &node, TpmSpec &spec);

std::size_t parse_translate(const pugi::xml_node &node, TpmSpec &spec);
std::size_t parse_union(const pugi::xml_node &node, TpmSpec &spec);

std::size_t parse_sdf(const pugi::xml_node &node, TpmSpec &spec);
std::pair<ExitCode, TpmSpec> parse_spec(const std::string &path);
} // namespace tpm

#endif /* end of include guard: SCENE_HPP_RZWYUXDC */
