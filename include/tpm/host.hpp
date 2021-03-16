#ifndef TPM_HOST_HPP_
#define TPM_HOST_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <uuid.h>
#if __has_include(<CL/opencl.hpp>)
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include <CL/opencl.hpp>
#else
#error "OpenCL C++ API is required for TPM GPU support"
#endif

#include "compiler.hpp"

namespace tpm::host {

enum DeviceType {
  DEFAULT = CL_DEVICE_TYPE_DEFAULT,
  CPU = CL_DEVICE_TYPE_CPU,
  GPU = CL_DEVICE_TYPE_GPU,
  ACCELERATOR = CL_DEVICE_TYPE_ACCELERATOR,
  CUSTOM = CL_DEVICE_TYPE_CUSTOM,
  ALL = CL_DEVICE_TYPE_ALL,
};

struct DeviceInfo {
  DeviceType type;
  uuids::uuid uuid;
  std::uint32_t max_compute_units;
  std::uint32_t max_clock_frequency;
  std::uint64_t max_memory;
  std::vector<std::string> il_version;
  bool available;
  bool compiler_available;
  std::string name;
  std::string vendor;
  std::string profile;
  std::string version;
};

struct PlatformInfo {
  std::vector<DeviceInfo> devices;
  uuids::uuid uuid;
  std::string profile;
  std::string version;
  std::string name;
  std::string vendor;
};

struct Info {
  std::vector<PlatformInfo> platforms;
};

class Host {
public:
  virtual ~Host() {}

  virtual bool valid() TPM_NOEXCEPT { return false; }
  virtual Info info() { return Info{std::vector<PlatformInfo>{}}; }

  virtual bool initialize(const std::vector<uuids::uuid> &) { return false; }
  virtual bool compile(const std::string &) { return false; }
  virtual bool compile(const std::vector<uuids::uuid> &, const std::string &) {
    return false;
  };

private:
};

class HostImpl : public Host {
public:
  struct PlatformData {
    cl::Platform platform;
    std::vector<uuids::uuid> devices;
    uuids::uuid context;
  };
  struct DeviceData {
    cl::Device device;
    uuids::uuid platform;
    uuids::uuid context;
    uuids::uuid queue;
  };
  struct ContextData {
    cl::Context context;
    uuids::uuid platform;
    std::vector<uuids::uuid> devices;
    std::vector<uuids::uuid> queues;
    uuids::uuid kernel;
  };
  struct CommandQueueData {
    cl::CommandQueue queue;
    uuids::uuid context, device;
  };
  struct KernelData {
    cl::Program program;
    cl::Kernel kernel;
    uuids::uuid context;
  };
  HostImpl();

  bool valid() TPM_NOEXCEPT TPM_OVERRIDE;
  Info info() TPM_OVERRIDE;

  bool initialize(const std::vector<uuids::uuid> &devices) TPM_OVERRIDE;
  bool compile(const std::vector<uuids::uuid> &platforms,
               const std::string &kernel) TPM_OVERRIDE;

private:
  std::unordered_map<uuids::uuid, PlatformData> platforms;
  std::unordered_map<uuids::uuid, DeviceData> devices;
  std::unordered_map<uuids::uuid, ContextData> contexts;
  std::unordered_map<uuids::uuid, CommandQueueData> queues;
  std::unordered_map<uuids::uuid, KernelData> kernels;
};

typedef HostImpl HostLocal;
} // namespace tpm::host

#endif /* end of include guard: TPM_HOST_HPP_ */
