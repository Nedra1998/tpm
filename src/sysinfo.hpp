#ifndef SYSINFO_HPP_NEJMPUIX
#define SYSINFO_HPP_NEJMPUIX

#include <chrono>
#include <thread>

#include "prof.hpp"

#define TPM_SYSINFO_MAX_CPU_COUNT 64

namespace tpm::sysinfo {
double calc_cpu_percentage(const std::uint32_t &user, const std::uint32_t &nice,
                           const std::uint32_t &sys, const std::uint32_t &idle,
                           const std::uint32_t &prev_user,
                           const std::uint32_t &prev_nice,
                           const std::uint32_t &prev_sys,
                           const std::uint32_t &prev_idle);
void logger();
void summary();

template <class Rep = std::int64_t, class Period = std::milli>
void setup_logger(const std::chrono::duration<Rep, Period> &sleep_duration =
                      std::chrono::milliseconds(50)) {
#ifndef NDEBUG
  std::thread([sleep_duration]() {
    while (true) {
      tpm::sysinfo::logger();
      std::this_thread::sleep_for(sleep_duration);
    }
  }).detach();
#endif
}
} // namespace tpm::sysinfo

#endif /* end of include guard: SYSINFO_HPP_NEJMPUIX */
