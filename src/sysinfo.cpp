#include "sysinfo.hpp"
#include <cstdint>
#include <limits>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#include "sys/sysinfo.h"
#include "sys/types.h"
#endif

#include <CL/sycl.hpp>
#include <fmt/format.h>

#include "log.hpp"
#include "prof.hpp"
#include "version.hpp"

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

double tpm::sysinfo::calc_cpu_percentage(
    const std::uint32_t &user, const std::uint32_t &nice,
    const std::uint32_t &sys, const std::uint32_t &idle,
    const std::uint32_t &prev_user, const std::uint32_t &prev_nice,
    const std::uint32_t &prev_sys, const std::uint32_t &prev_idle) {
  if (user < prev_user || nice < prev_nice || sys < prev_sys ||
      idle < prev_idle)
    return -1.0;
  std::uint32_t total_used =
      (user - prev_user) + (nice - prev_nice) + (sys - prev_sys);
  std::uint32_t total = total_used + (idle - prev_idle);
  return 100.0 * static_cast<double>(total_used) / static_cast<double>(total);
}

void tpm::sysinfo::logger() {
  PFUNC();

  static thread_local std::uint32_t cpu_prev_user, cpu_prev_nice, cpu_prev_sys,
      cpu_prev_idle;

  struct sysinfo mem_info;
  ::sysinfo(&mem_info);
  std::uint64_t virtual_mem_total =
      (mem_info.totalram + mem_info.totalswap) * mem_info.mem_unit;
  std::uint64_t virtual_mem_used = ((mem_info.totalram - mem_info.freeram) +
                                    (mem_info.totalswap - mem_info.freeswap)) *
                                   mem_info.mem_unit;
  double virtual_mem_perc = 100.0 * static_cast<double>(virtual_mem_used) /
                            static_cast<double>(virtual_mem_total);

  PVAR("memory", virtual_mem_perc);

  double cpu_perc;
  FILE *file = fopen("/proc/stat", "r");
  if (file == nullptr) {
    LWARN("Failed to read \"/proc/stat\", cannot report CPU utilization");
  } else {

    std::uint32_t user, nice, sys, idle;

    if (fscanf(file, "cpu %u %u %u %u", &user, &nice, &sys, &idle) == 4) {
      cpu_perc =
          calc_cpu_percentage(user, nice, sys, idle, cpu_prev_user,
                              cpu_prev_nice, cpu_prev_sys, cpu_prev_idle);
      PVAR("cpu", cpu_perc);

      cpu_prev_user = user;
      cpu_prev_nice = nice;
      cpu_prev_sys = sys;
      cpu_prev_idle = idle;
    }

    fclose(file);
  }
  LDEBUG("CPU: {:6.2f}%, MEMORY: {:6.2f}%", cpu_perc, virtual_mem_perc);
  if (cpu_perc > 90.0)
    LWARN("High CPU utilization {:6.2f}%", cpu_perc);
  if (virtual_mem_perc > 80.0)
    LWARN("High memory utilization {:6.2f}%", virtual_mem_perc);
}

void tpm::sysinfo::summary() {
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
