#include <CL/sycl.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>
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

#include "log.hpp"
#include "version.hpp"

void print_info();

int main(int argc, const char **argv) {

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

  auto result = options.parse(argc, argv);
  if (result.count("help") != 0) {
    std::cout << options.help() << std::endl;
    return 0;
  } else if (result.count("version") != 0) {
    std::cout << tpm::version::semver << std::endl;
    return 0;
  } else if (result.count("info") != 0) {
    print_info();
    return 0;
  }

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
