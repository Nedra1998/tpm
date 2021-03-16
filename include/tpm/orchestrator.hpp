#ifndef TPM_ORCHESTRATOR_HPP_
#define TPM_ORCHESTRATOR_HPP_

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <uuid.h>

#include "compiler.hpp"
#include "host.hpp"

namespace tpm {
class Orchestrator {
public:
  struct DeviceInfo {
    uuids::uuid uuid;
    std::size_t mem, ops;
    bool spirv_support;
  };
  struct PlatformInfo {
    uuids::uuid uuid;
    std::shared_ptr<host::Host> host_ptr;
    std::vector<DeviceInfo> devices;
  };

  void push_back(const std::shared_ptr<host::Host> &host) TPM_NOEXCEPT;

  bool initialize() TPM_NOEXCEPT;
  bool compile(const std::string& kernel);

private:
  std::vector<Orchestrator::PlatformInfo> platforms;
};
} // namespace tpm

#endif /* end of include guard: TPM_ORCHESTRATOR_HPP_ */
