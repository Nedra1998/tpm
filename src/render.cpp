#include "render.hpp"

#include <cmath>
#include <filesystem>
#include <limits>
#include <random>
#include <vector>

#include <CL/sycl.hpp>
#include <hipSYCL/sycl/handler.hpp>
#include <hipSYCL/sycl/queue.hpp>

#include "log.hpp"
#include "prof.hpp"
#include "sdf.hpp"
#include "stb_image_write.h"

constexpr float epsilon = std::numeric_limits<float>::epsilon() * 10.0f;
constexpr float max_t = 100.0f;
constexpr std::size_t sample_count = 2;

namespace fmt {
template <typename T, int N> struct formatter<cl::sycl::vec<T, N>> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const cl::sycl::vec<T, N> &r, FormatContext &ctx) {
    if constexpr (N == 1) {
      return format_to(ctx.out(), "{{{}}}", r[0]);
    } else if constexpr (N == 2) {
      return format_to(ctx.out(), "{{{}, {}}}", r[0], r[1]);
    } else if constexpr (N == 3) {
      return format_to(ctx.out(), "{{{}, {}, {}}}", r[0], r[1], r[2]);
    } else if constexpr (N == 4) {
      return format_to(ctx.out(), "{{{}, {}, {}, {}}}", r[0], r[1], r[2], r[3]);
    } else if constexpr (N == 8) {
      return format_to(ctx.out(), "{{{}, {}, {}, {}, {}, {}, {}, {}}}", r[0],
                       r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
    } else if constexpr (N == 16) {
      return format_to(
          ctx.out(),
          "{{{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}}}",
          r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10],
          r[11], r[12], r[13], r[14], r[15]);
    }
  }
};

} // namespace fmt

float tpm::eval_sdf(
    const cl::sycl::float3 &p, const std::size_t &id,
    const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdf,
    std::size_t &mat) {
  if (sdf[id].mat != std::numeric_limits<std::size_t>::max())
    mat = sdf[id].mat;
  float t = std::numeric_limits<float>::infinity();
  switch (sdf[id].type) {
  case SPHERE:
    t = sdf::sphere(p, sdf[id].args[0]);
    break;
  case TRANSLATE:
    t = eval_sdf(
        sdf::op_translate(p, cl::sycl::float3(sdf[id].args[0], sdf[id].args[1],
                                              sdf[id].args[2])),
        sdf[id].a, sdf, mat);
    break;
  case UNION: {
    std::size_t ma = mat, mb = mat;
    float a = eval_sdf(p, sdf[id].a, sdf, ma);
    float b = eval_sdf(p, sdf[id].b, sdf, mb);
    if (b > a) {
      mat = ma;
      t = a;
    } else {
      mat = mb;
      t = b;
    }
    break;
  }
  }
  return t;
}

cl::sycl::float3 tpm::ray_march(
    const cl::sycl::float3 &p, const cl::sycl::float3 &d,
    const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdfs,
    const cl::sycl::accessor<Mat, 1, cl::sycl::access::mode::read> &mats) {
  float t = 0.0f, delta_t = std::numeric_limits<float>::infinity();
  std::size_t mat = std::numeric_limits<std::size_t>::max();
  while (t < max_t && delta_t > epsilon) {
    delta_t = eval_sdf(p + (t * d), 0, sdfs, mat);
    t += delta_t;
  }
  if (delta_t <= epsilon && mat != std::numeric_limits<std::size_t>::max()) {
    Mat it = mats[mat];
    switch (it.type) {
    case EMISSION:
    case DIFFUSE:
    case GLASS:
    case GLOSSY:
      return it.color;
    case NONE:
    default:
      break;
    }
  }
  return cl::sycl::float3(0.0, 0.0, 0.0);
}

cl::sycl::float3 tpm::render_pixel(
    const cl::sycl::uint4 &pixel, cl::sycl::uint4 &seed,
    const cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> &sdfs,
    const cl::sycl::accessor<Mat, 1, cl::sycl::access::mode::read> &mats) {
  cl::sycl::float3 color(0.0, 0.0, 0.0);
  cl::sycl::float3 pos(0.0, 0.0, 0.0);
  cl::sycl::float3 scaling(1.0 / static_cast<float>(pixel[2]),
                           1.0 / static_cast<float>(pixel[3]), 1.0);
  cl::sycl::float3 translate(-0.5, -0.5, 0.0);
  cl::sycl::float3 dir(static_cast<float>(pixel[0]),
                       static_cast<float>(pixel[1]), 1.0);

  dir = (dir * scaling) + translate;

  for (std::size_t i = 0; i < sample_count; ++i) {
    cl::sycl::float3 jiggle(random(seed) - 0.5, random(seed) - 0.5, 0.0);
    cl::sycl::float3 res = ray_march(pos, dir + (jiggle * scaling), sdfs, mats);

    color += res / sample_count;
  }
  return color;
}

tpm::ExitCode tpm::render_frame(const TpmSpec &spec) {
  PFUNC(&spec);

  cl::sycl::queue queue;
  Image img(spec.image.width, spec.image.height, spec.image.tile);

  {
    PSCOPE("RenderKernel", img.size, img.tile_size());
    cl::sycl::uint3 img_size = img.size;
    cl::sycl::uint2 tile_size = img.tile_size();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint32_t> dist;

    std::vector<cl::sycl::uint4> seeds;
    for (std::size_t i = 0; i < tile_size[0] * tile_size[1]; ++i)
      seeds.push_back(
          cl::sycl::uint4(dist(gen), dist(gen), dist(gen), dist(gen)));

    cl::sycl::buffer<cl::sycl::float3> img_buffer(img.buffer.data(),
                                                  img.buffer.size());
    cl::sycl::buffer<cl::sycl::uint4> seeds_buffer(seeds.data(), seeds.size());
    cl::sycl::buffer<Sdf> sdfs_buffer(spec.sdfs.data(), spec.sdfs.size());
    cl::sycl::buffer<Mat> mats_buffer(spec.mats.data(), spec.mats.size());

    queue.submit([&](cl::sycl::handler &cgh) {
      cl::sycl::accessor<cl::sycl::float3, 1, cl::sycl::access::mode::write>
          buffer_ptr =
              img_buffer.get_access<cl::sycl::access::mode::write>(cgh);
      cl::sycl::accessor<cl::sycl::uint4, 1, cl::sycl::access::mode::read>
          seeds_ptr =
              seeds_buffer.get_access<cl::sycl::access::mode::read>(cgh);
      cl::sycl::accessor<Sdf, 1, cl::sycl::access::mode::read> sdfs_ptr =
          sdfs_buffer.get_access<cl::sycl::access::mode::read>(cgh);
      cl::sycl::accessor<Mat, 1, cl::sycl::access::mode::read> mats_ptr =
          mats_buffer.get_access<cl::sycl::access::mode::read>(cgh);

      cgh.parallel_for(
          cl::sycl::range<2>(tile_size[0], tile_size[1]),
          [=](cl::sycl::item<2> item) {
            cl::sycl::uint4 tile =
                Image::tile(img_size, cl::sycl::uint2(item[0], item[1]));
            cl::sycl::uint4 seed = seeds_ptr[item.get_linear_id()];

            PFUNC(tile, seed);

            for (std::uint32_t x = tile[0]; x < tile[2]; ++x) {
              for (std::uint32_t y = tile[1]; y < tile[3]; ++y) {
                buffer_ptr[Image::idx(img_size, cl::sycl::uint2(x, y))] =
                    render_pixel(
                        cl::sycl::uint4(x, y, img_size[0], img_size[1]), seed,
                        sdfs_ptr, mats_ptr);
              }
            }
          });
    });
  }

  write(spec.image.path, img);

  return OK;
}

tpm::ExitCode tpm::write(const std::filesystem::path &path, const Image &img) {
  PFUNC(path.string(), &img);

  if (!path.parent_path().empty() &&
      !std::filesystem::exists(path.parent_path())) {
    LINFO("Creating output directory \"{}\"", path.parent_path().string());
    if (!std::filesystem::create_directories(path.parent_path())) {
      LWARN("Failed to create output directory \"{}\", aborting image write",
            path.parent_path().string());
      return MKDIR_ERROR;
    }
  }

  if (path.extension() == ".png") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.size[0] * img.size[0] * 3 * sizeof(std::uint8_t)));
    for (std::size_t i = 0; i < img.size[0] * img.size[0]; ++i) {
      std::size_t idx = 3 * i;
      data[idx + 0] = static_cast<std::uint8_t>(img.buffer[i][0] * 255.0f);
      data[idx + 1] = static_cast<std::uint8_t>(img.buffer[i][1] * 255.0f);
      data[idx + 2] = static_cast<std::uint8_t>(img.buffer[i][2] * 255.0f);
    }

    bool ret =
        (stbi_write_png(path.c_str(), static_cast<int>(img.size[0]),
                        static_cast<int>(img.size[1]), static_cast<int>(3),
                        data, static_cast<int>(img.size[0] * 3)) != 0);
    free(data);

    if (!ret)
      return IMG_WRITE_ERR;
  } else if (path.extension() == ".jpg" || path.extension() == "jpeg" ||
             path.extension() == "jpe") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.size[0] * img.size[0] * 3 * sizeof(std::uint8_t)));
    for (std::size_t i = 0; i < img.size[0] * img.size[0]; ++i) {
      std::size_t idx = 3 * i;
      data[idx + 0] = static_cast<std::uint8_t>(img.buffer[i][0] * 255.0f);
      data[idx + 1] = static_cast<std::uint8_t>(img.buffer[i][1] * 255.0f);
      data[idx + 2] = static_cast<std::uint8_t>(img.buffer[i][2] * 255.0f);
    }

    bool ret = (stbi_write_jpg(path.c_str(), static_cast<int>(img.size[0]),
                               static_cast<int>(img.size[1]),
                               static_cast<int>(3), data, 100) != 0);
    free(data);

    if (!ret)
      return IMG_WRITE_ERR;
  } else if (path.extension() == ".bmp") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.size[0] * img.size[0] * 3 * sizeof(std::uint8_t)));
    for (std::size_t i = 0; i < img.size[0] * img.size[0]; ++i) {
      std::size_t idx = 3 * i;
      data[idx + 0] = static_cast<std::uint8_t>(img.buffer[i][0] * 255.0f);
      data[idx + 1] = static_cast<std::uint8_t>(img.buffer[i][1] * 255.0f);
      data[idx + 2] = static_cast<std::uint8_t>(img.buffer[i][2] * 255.0f);
    }

    bool ret = (stbi_write_bmp(path.c_str(), static_cast<int>(img.size[0]),
                               static_cast<int>(img.size[1]),
                               static_cast<int>(3), data) != 0);
    free(data);

    if (!ret)
      return IMG_WRITE_ERR;
  } else if (path.extension() == ".hdr") {
    bool ret = (stbi_write_hdr(
                    path.c_str(), static_cast<int>(img.size[0]),
                    static_cast<int>(img.size[1]), static_cast<int>(3),
                    reinterpret_cast<const float *>(img.buffer.data())) != 0);
    if (!ret)
      return IMG_WRITE_ERR;
  }

  return OK;
}
