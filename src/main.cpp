#include <CL/sycl.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <hipSYCL/sycl/device.hpp>
#include <hipSYCL/sycl/handler.hpp>
#include <hipSYCL/sycl/info/device.hpp>
#include <hipSYCL/sycl/info/platform.hpp>
#include <hipSYCL/sycl/platform.hpp>
#include <hipSYCL/sycl/queue.hpp>
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
#include "render.hpp"
#include "sysinfo.hpp"

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
    tpm::sysinfo::summary();
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
  PBEGIN("SysInfo");
  tpm::sysinfo::setup_logger();
  PEND("SysInfo");
  PEND("Setup");

  if (exit_code == 0) {
    cl::sycl::queue q;

    tpm::image::Image img(3840, 2160, 32, tpm::image::COLOR);
    tpm::render(q, img);

    tpm::image::write("output.png", img);
  }

  PSTOP();
  return 0;
}
