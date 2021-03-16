#include "host.hpp"
#include "magic_enum.hpp"

#include <cstdint>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/ranges.h>
#include <uuid.h>
#if __has_include(<CL/opencl.hpp>)
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include <CL/opencl.hpp>
#else
#error "OpenCL C++ API is required for TPM GPU support"
#endif

#include "compiler.hpp"
#include "logging.hpp"

static std::random_device rd;
static std::mt19937 rng(rd());
static uuids::uuid_random_generator uuid_random{rng};

static uuids::uuid_name_generator uuid_generator(uuid_random());

tpm::host::HostImpl::HostImpl() {}

bool tpm::host::HostImpl::valid() TPM_NOEXCEPT { return true; }

tpm::host::Info tpm::host::HostImpl::info() {
  Info response;

  std::vector<cl::Platform> all_platforms;
  if (cl::Platform::get(&all_platforms) != CL_SUCCESS ||
      all_platforms.empty()) {
    LWARN("tpm::host", "No OpenCL platforms found");
    return response;
  }

  for (std::size_t i = 0; i < all_platforms.size(); ++i) {
    const auto &p = all_platforms[i];
    PlatformInfo pinfo;
    pinfo.name = p.getInfo<CL_PLATFORM_NAME>();
    pinfo.profile = p.getInfo<CL_PLATFORM_PROFILE>();
    pinfo.vendor = p.getInfo<CL_PLATFORM_VENDOR>();
    pinfo.version = p.getInfo<CL_PLATFORM_VERSION>();
    pinfo.uuid = uuid_generator(pinfo.name);

    PlatformData pdata;
    pdata.platform = p;

    LTRACE("tpm::host",
           "cl::Platform({}) {{name={}, profile={}, vendor={}, version={}}}",
           pinfo.uuid, pinfo.name, pinfo.profile, pinfo.vendor, pinfo.version);

    std::vector<cl::Device> pdevices;
    if (p.getDevices(CL_DEVICE_TYPE_ALL, &pdevices) != CL_SUCCESS ||
        pdevices.empty()) {
      LWARN("tpm::host", "OpenCL Platform ({}) has no devices",
            p.getInfo<CL_PLATFORM_NAME>());
      response.platforms.push_back(pinfo);
      continue;
    }

    for (std::size_t j = 0; j < pdevices.size(); ++j) {
      const auto &d = pdevices[j];
      DeviceInfo dinfo;

      dinfo.type = magic_enum::enum_cast<DeviceType>(
                       static_cast<unsigned int>(d.getInfo<CL_DEVICE_TYPE>()))
                       .value_or(DeviceType::DEFAULT);

      dinfo.max_compute_units = d.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
      dinfo.max_clock_frequency = d.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
      dinfo.max_memory = d.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

      std::istringstream il_versions(d.getInfo<CL_DEVICE_IL_VERSION>());
      dinfo.il_version = std::vector<std::string>{
          std::istream_iterator<std::string>{il_versions},
          std::istream_iterator<std::string>{}};

      dinfo.available = d.getInfo<CL_DEVICE_AVAILABLE>();
      dinfo.compiler_available = d.getInfo<CL_DEVICE_COMPILER_AVAILABLE>();
      dinfo.name = d.getInfo<CL_DEVICE_NAME>();
      dinfo.vendor = d.getInfo<CL_DEVICE_VENDOR>();
      dinfo.profile = d.getInfo<CL_DEVICE_PROFILE>();
      dinfo.version = d.getInfo<CL_DEVICE_VERSION>();
      dinfo.uuid = uuid_generator(dinfo.name);

      LTRACE("tpm::host",
             "cl::Device({}) {{type={}, max_compute_units={}, "
             "max_clock_frequency={}, max_memory={}, il_version={}, "
             "available={}, compiler_available={}, name={}, vendor={}, "
             "profile={}, version={} }}",
             uuids::to_string(dinfo.uuid), magic_enum::enum_name(dinfo.type),
             dinfo.max_compute_units, dinfo.max_clock_frequency,
             dinfo.max_memory, dinfo.il_version, dinfo.available,
             dinfo.compiler_available, dinfo.name, dinfo.vendor, dinfo.profile,
             dinfo.version);

      devices.insert({dinfo.uuid, {d, pinfo.uuid}});
      pinfo.devices.push_back(dinfo);
      pdata.devices.push_back(dinfo.uuid);
    }

    platforms.insert({pinfo.uuid, pdata});
    response.platforms.push_back(pinfo);
  }

  return response;
}

bool tpm::host::HostImpl::initialize(
    const std::vector<uuids::uuid> &init_devices) {
  bool result = false;
  std::unordered_map<uuids::uuid, std::vector<uuids::uuid>> platform_device_map;

  for (auto &id : init_devices) {
    std::unordered_map<uuids::uuid, std::vector<uuids::uuid>>::iterator it;
    if ((it = platform_device_map.find(devices[id].platform)) !=
        platform_device_map.end()) {
      it->second.push_back(id);
    } else {
      platform_device_map.insert({devices[id].platform, {id}});
    }
  }

  for (auto &it : platform_device_map) {
    std::vector<cl::Device> context_devices;
    for (auto &dev : it.second)
      context_devices.push_back(devices[dev].device);
    uuids::uuid context_uuid = uuid_random();
    ContextData cdata{cl::Context(context_devices), it.first, it.second};
    LTRACE("tpm::host", "cl::Context({}) {{platform={}, devices={} }}",
           context_uuid, it.first, it.second);
    platforms[it.first].context = context_uuid;
    for (auto &dev : it.second) {
      devices[dev].context = context_uuid;

      uuids::uuid queue_uuid = uuid_random();
      CommandQueueData qdata{
          cl::CommandQueue(cdata.context, devices[dev].device), context_uuid,
          dev};
      LTRACE("tpm::host", "cl::CommandQueue({}) {{context={}, device={}}}",
             queue_uuid, context_uuid, dev);
      queues.insert({queue_uuid, qdata});

      devices[dev].queue = queue_uuid;
      cdata.queues.push_back(queue_uuid);
    }
    contexts.insert({context_uuid, cdata});
    result |= true;
  }

  return result;
}

bool tpm::host::HostImpl::compile(
    const std::vector<uuids::uuid> &compile_platforms,
    const std::string &kernel) {
  bool result = false;

  for (auto &it : compile_platforms) {
    std::unordered_map<uuids::uuid, ContextData>::iterator ctx =
        contexts.find(platforms[it].context);

    cl::Program program(ctx->second.context, kernel);
    try {
      program.build("-cl-std=CL2.0");
    } catch (...) {
      LERR("tpm::host", "Failed to compile kernel on for context {}",
           ctx->first);
      cl_int build_err = CL_SUCCESS;
      auto build_info = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&build_err);
      for (auto &pair : build_info) {
        LERR("tpm::host", "{}: {}",
             uuid_generator(pair.first.getInfo<CL_DEVICE_NAME>()), pair.second);
      }
      continue;
    }

    uuids::uuid kernel_uuid = uuid_random();
    KernelData kdata{program, cl::Kernel(program, "tpm"), ctx->first};
    ctx->second.kernel = kernel_uuid;

    std::string compressed_kernel = kernel;
    std::replace(compressed_kernel.begin(), compressed_kernel.end(), '\n', ' ');

    LTRACE("tpm::host", "cl::Kernel({}) {{context={}, source=\"{}\"}}", kernel_uuid,
           ctx->first, compressed_kernel);
    kernels.insert({kernel_uuid, kdata});
    result |= true;
  }
  return result;
}
