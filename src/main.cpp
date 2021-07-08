#include <CL/sycl.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <hipSYCL/sycl/device.hpp>
#include <hipSYCL/sycl/info/device.hpp>
#include <hipSYCL/sycl/info/platform.hpp>
#include <hipSYCL/sycl/platform.hpp>
#include <iostream>
#include <limits>
#include <ostream>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

#if defined(_WIN32) || defined(_WIN64)
#define NOMINMAX
#include <io.h>
#include <windows.h>
#undef min
#undef max
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#include <unistd.h>
#endif

#define PL_IMPLEMENTATION 1
#include "image.hpp"
#include "log.hpp"
#include "prof.hpp"
#include "version.hpp"

void print_info();

using data_type = float;

std::vector<data_type> add(cl::sycl::queue &q, const std::vector<data_type> &a,
                           const std::vector<data_type> &b) {
  PFUNC(&q, a, b);
  std::vector<data_type> c(a.size());

  assert(a.size() == b.size());
  cl::sycl::range<1> work_items{a.size()};

  {
    cl::sycl::buffer<data_type> buff_a(a.data(), a.size());
    cl::sycl::buffer<data_type> buff_b(b.data(), b.size());
    cl::sycl::buffer<data_type> buff_c(c.data(), c.size());

    q.submit([&](cl::sycl::handler &cgh) {
      auto access_a = buff_a.get_access<cl::sycl::access::mode::read>(cgh);
      auto access_b = buff_b.get_access<cl::sycl::access::mode::read>(cgh);
      auto access_c = buff_c.get_access<cl::sycl::access::mode::write>(cgh);

      cgh.parallel_for<class vector_add>(work_items, [=](cl::sycl::id<1> tid) {
        access_c[tid] = access_a[tid] + access_b[tid];
      });
    });
  }
  return c;
}

int main(int argc, const char **argv) {
  PINIT("tpm");

  PBEGIN("Setup");
  PBEGIN("Argparse");
  cxxopts::Options options(
      "TPM", fmt::format("TPM v{}.{} - C++ Minimal Path Marcher",
                         tpm::version::major, tpm::version::minor));

  // clang-format off
  options.add_options()
    ("h,help", "Display usage information")
    ("v,verbose", "Enable verbose logging")
    ("V,version", "Display detailed version information")
    ("I,info", "Display detailed application information");
  // clang-format on

  int exit_code = 0;
  auto result = options.parse(argc, argv);
  PVAR("help", result.count("help"));
  PVAR("verbose", result.count("verbose"));
  PVAR("version", result.count("version"));
  PVAR("info", result.count("info"));
  PEND("Argparse");

  if (result.count("help") != 0) {
    std::cout << options.help() << std::endl;
    exit_code = -1;
  } else if (result.count("version") != 0) {
    std::cout << tpm::version::semver << std::endl;
    exit_code = -1;
  } else if (result.count("info") != 0) {
    print_info();
    exit_code = -1;
  }

  PBEGIN("Logging");
  try {
    spdlog::level::level_enum log_level = spdlog::level::warn;
    if (result.count("verbose") != 0) {
#ifdef NDEBUG
      log_level = spdlog::level::info;
#else
      log_level = spdlog::level::trace;
#endif
    }

    tpm::logging::dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
    {
#if defined(_WIN32) || defined(_WIN64)
      bool is_tty = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
      bool is_tty = ::isatty(fileno(stdout)) != 0;
#else
      bool is_tty = false;
#endif
      if (is_tty) {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_level(log_level);
        tpm::logging::dist_sink->add_sink(sink);
      } else {
        auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        sink->set_level(log_level);
        tpm::logging::dist_sink->add_sink(sink);
      }
    }
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return -1;
  }
  PEND("Logging");
  PEND("Setup");

  if (exit_code == 0) {
    PSCOPE("Calculations");
    PBEGIN("Initialize");
    PBEGIN("Queue");
    cl::sycl::queue q;
    PEND("Queue");
    PBEGIN("Vectors");
    std::vector<data_type> a = {1.f, 2.f, 3.f, 4.f, 5.f};
    std::vector<data_type> b = {-1.f, 2.f, -3.f, 4.f, -5.f};
    PEND("Vectors");
    PEND("Initialize");
    auto res = add(q, a, b);
    a = add(q, b, res);
    b = add(q, res, a);
    res = add(q, a, res);

    {
      PSCOPE("IO");
      std::cout << "Result: " << std::endl;
      for (const auto x : res)
        std::cout << x << std::endl;
    }
  }

  if (exit_code == 0) {
    PSCOPE("Render");
    tpm::image::Image img(500, 500, tpm::image::COLOR);
    for (std::size_t i = 0; i < img.tile_count(); ++i) {
      tpm::image::Tile tile = img.get_tile(i);

      float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

      for (std::size_t x = 0; x < tile.width; ++x)
        for (std::size_t y = 0; y < tile.height; ++y)
          tile.set(tpm::image::COLOR, x, y, r, g, b);
      img.merge_tile(tile);
    }

    tpm::image::write("output-{buffer}.png", img);
  }

  PSTOP();
  return 0;
}

#define INFO_HEADER(label) fmt::print("{}:\n", label);
#define INFO_FIELD1(key, value, ...)                                           \
  fmt::print("  {:20}  {}\n", fmt::format("{}:", key),                         \
             fmt::format(value, ##__VA_ARGS__));
#define INFO_FIELD2(key, value, ...)                                           \
  fmt::print("    {:18}  {}\n", fmt::format("{}:", key),                       \
             fmt::format(value, ##__VA_ARGS__));
#define INFO_FIELD3(key, value, ...)                                           \
  fmt::print("      {:16}  {}\n", fmt::format("{}:", key),                     \
             fmt::format(value, ##__VA_ARGS__));
#define INFO_FIELD(...) INFO_FIELD1(__VA_ARGS__)
#define INFO_SUB_FIELD(...) INFO_FIELD2(__VA_ARGS__)
#define INFO_SUB_SUB_FIELD(...) INFO_FIELD3(__VA_ARGS__)

void print_info() {
  PFUNC();
  INFO_HEADER("Application");
  INFO_FIELD("TPM Version", tpm::version::semver);

  INFO_HEADER("Compilation");
  INFO_FIELD("C++ Version", "C++{}.{} ({})", ((__cplusplus / 100L) % 100),
             (__cplusplus % 100), __cplusplus);

#if defined(__clang__) && defined(__apple_build_version)
  INFO_FIELD("Compiler", "AppleClang v{}.{}.{}", __clang_major__,
             __clang_minor__, __clang_patchlevel__);
#elif defined(__clang__)
  INFO_FIELD("Compiler", "Clang v{}.{}.{}", __clang_major__, __clang_minor__,
             __clang_patchlevel__);
#elif defined(__GNUC__)
#if defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
  INFO_FIELD("Compiler", "GNU v{}.{}.{}", __GNUC__, __GNUC_MINOR__,
             __GNUC_PATCHLEVEL__);
#elif defined(__GNUC_MINOR__)
  INFO_FIELD("Compiler", "GNU v{}.{}", __GNUC__, __GNUC_MINOR__);
#else
  INFO_FIELD("Compiler", "GNU v{}", __GNUC__);
#endif
#elif defined(_MSC_VER)
#if defined(_MSC_FULL_VER)
#if _MSC_VER >= 1400
  INFO_FIELD("Compiler", "MSVC v{}.{}.{}", _MSC_VER / 100, _MSC_VER % 100,
             _MSC_FULL_VERR % 100000);
#else
  INFO_FIELD("Compiler", "v{}.{}.{}", _MSC_VER / 100, _MSC_VER % 100,
             _MSC_FULL_VERR % 10000);
#endif
#else
  INFO_FIELD("Compiler", "MSVC v{}.{}", _MSC_VER / 100, _MSC_VER % 100);
#endif
#else
  INFO_FIELD("Compiler", "Unknown");
#endif

#if defined(__TIMESTAMP__)
  INFO_FIELD("Date & Time", "{}", __TIMESTAMP__);
#else
  INFO_FIELD("Date & Time", "{} {}", __DATE__, __TIME__);
#endif

#if defined(_WIN32) || defined(_WIN64)
  INFO_FIELD("Operating System", "Windows");
#elif defined(__APPLE__)
  INFO_FIELD("Operating System", "MacOS");
#elif defined(__unix__) || defined(__unix)
  INFO_FIELD("Operating System", "Linux");
#else
  INFO_FIELD("Operating System", "Unknown");
#endif

  INFO_HEADER("SYCL");
#ifdef SYCL_LANGUAGE_VERSION
  INFO_FIELD("SYCL Version", "SYCL {}.{} ({})",
             ((SYCL_LANGUAGE_VERSION / 100L) % 100),
             (SYCL_LANGUAGE_VERSION % 100), SYCL_LANGUAGE_VERSION);
#else
  INFO_FIELD("SYCL Version", "None");
#endif

  std::vector<cl::sycl::platform> platforms =
      cl::sycl::platform::get_platforms();
  for (const cl::sycl::platform &platform : platforms) {
    INFO_FIELD(platform.get_info<cl::sycl::info::platform::name>(), "");
    std::vector<cl::sycl::device> devices = platform.get_devices();
    for (const cl::sycl::device &device : devices) {
      INFO_SUB_FIELD(device.get_info<cl::sycl::info::device::name>(),
                     device.get_info<cl::sycl::info::device::version>());
      INFO_SUB_SUB_FIELD(
          "Compute Units", "{}",
          device.get_info<cl::sycl::info::device::max_compute_units>());
      double mem =
          static_cast<double>(
              device.get_info<cl::sycl::info::device::global_mem_size>()) /
          1000000000.0;
      INFO_SUB_SUB_FIELD(
          "Memory Size", "{:.1f}GB",
          mem < 10000 ? mem : std::numeric_limits<double>::quiet_NaN());
      double freq =
          device.get_info<cl::sycl::info::device::max_clock_frequency>() /
          1000.0;
      INFO_SUB_SUB_FIELD(
          "Clock Frequency", "{:.2f}GHz",
          freq > 0.01 ? freq : std::numeric_limits<double>::quiet_NaN());
    }
  }
}
