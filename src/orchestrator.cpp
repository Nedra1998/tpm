#include "orchestrator.hpp"

#include <cmath>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <uuid.h>

#include "compiler.hpp"
#include "host.hpp"
#include "logging.hpp"

static const std::array<std::string, 6> byte_suffix = {"B",  "KB", "MB",
                                                       "GB", "TB", "PB"};

void tpm::Orchestrator::push_back(const std::shared_ptr<host::Host> &host)
    TPM_NOEXCEPT {
  if (!host->valid())
    return;
  host::Info info = host->info();
  for (auto &p : info.platforms) {
    PlatformInfo pinfo{p.uuid, host};
    for (auto &d : p.devices) {
      if (!d.available || !d.compiler_available) {
        LDEBUG("tpm::orchestrator", "Ignoring unavailable device {}", d.uuid);
      } else {
        pinfo.devices.push_back(
            {d.uuid, d.max_memory, d.max_compute_units * d.max_clock_frequency,
             std::any_of(
                 d.il_version.begin(), d.il_version.end(),
                 [](std::string &il) { return il.starts_with("SPIR-V"); })});

        std::uint8_t place = static_cast<std::uint8_t>(std::floor(
            (std::log(static_cast<float>(d.max_memory)) / std::log(1024.0f))));
        float num = static_cast<float>(d.max_memory) /
                    std::pow(1024.0f, static_cast<float>(place));
        LDEBUG("tpm::orchestrator", "Registered device {} {}@{}GHz {:.1f}{} {}",
               uuids::to_string(d.uuid), d.max_compute_units,
               static_cast<float>(d.max_clock_frequency) / 1000.0f, num,
               byte_suffix[place],
               pinfo.devices.back().spirv_support ? "[SPIR-V]" : "");
      }
    }
    platforms.push_back(pinfo);
  }
}

bool tpm::Orchestrator::initialize() TPM_NOEXCEPT {
  bool response = false;

  for (auto &it : platforms) {
    std::vector<uuids::uuid> platform_devices(it.devices.size());
    std::transform(it.devices.begin(), it.devices.end(),
                   platform_devices.begin(),
                   [](const DeviceInfo &dinfo) { return dinfo.uuid; });
    response |= it.host_ptr->initialize(platform_devices);
  }
  return response;
}

bool tpm::Orchestrator::compile(const std::string &kernel) {
  bool response = false;
  for (auto &it : platforms) {
    response |= it.host_ptr->compile({it.uuid}, kernel);
  }
  return response;
}
