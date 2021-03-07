#include "gpu.hpp"

#include <CL/cl.h>
#include <CL/opencl.hpp>
#include <vector>

#include <magic_enum.hpp>

#include "logging.hpp"

bool tpm::gpu::GPULoadBalancer::initialize() {
  std::vector<cl::Platform> platforms;

  if (cl::Platform::get(&platforms) != CL_SUCCESS || platforms.empty()) {
    LERR("tpm::gpu", "No OpenCL platforms found");
    return false;
  }

  for (auto &p : platforms) {
    LTRACE("tpm::gpu",
           "OpenCL Platform: {{\"name\":\"{}\", \"profile\":\"{}\", "
           "\"vendor\":\"{}\", \"version\":\"{}\"}}",
           p.getInfo<CL_PLATFORM_NAME>(), p.getInfo<CL_PLATFORM_PROFILE>(),
           p.getInfo<CL_PLATFORM_VENDOR>(), p.getInfo<CL_PLATFORM_VERSION>());
    std::vector<cl::Device> platform_devices;
    if (p.getDevices(CL_DEVICE_TYPE_ALL, &platform_devices) != CL_SUCCESS ||
        platform_devices.empty()) {
      LERR("tpm::gpu", "OpenCL Platform has no devices");
    }
    std::shared_ptr<cl::Context> context =
        std::make_shared<cl::Context>(platform_devices);
    std::shared_ptr<cl::Program> program =
        std::make_shared<cl::Program>(*context, kernel);

    std::shared_ptr<cl::Image2D> img = std::make_shared<cl::Image2D>(
        *context, CL_MEM_READ_WRITE, cl::ImageFormat(CL_RGBA, CL_FLOAT), 500,
        500);

    try {
      program->build("-cl-std=CL2.0");
    } catch (...) {
      LERR("tpm::gpu", "Errors for failed build for all devices");
      cl_int build_err = CL_SUCCESS;
      auto build_info = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(&build_err);
      for (auto &pair : build_info) {
        LERR("tpm::gpu", "{}: {}", pair.first.getInfo<CL_DEVICE_NAME>(),
             pair.second);
      }
      continue;
    }

    for (auto &d : platform_devices) {
      cl_device_type type = d.getInfo<CL_DEVICE_TYPE>();
      std::size_t device_hash = std::hash<std::string>{}(
          p.getInfo<CL_PLATFORM_NAME>() + d.getInfo<CL_DEVICE_NAME>());
      LTRACE("tpm::gpu",
             "OpenCL Device({:X}): {{\"name\":\"{}\", \"image_support\": {}, "
             "\"profile\":\"{}\", \"vendor\":\"{}\", \"version\":\"{}\", "
             "\"type\":\"{}\"}}",
             device_hash,
             d.getInfo<CL_DEVICE_NAME>(),
             static_cast<bool>(d.getInfo<CL_DEVICE_IMAGE_SUPPORT>()),
             d.getInfo<CL_DEVICE_PROFILE>(), d.getInfo<CL_DEVICE_VENDOR>(),
             d.getInfo<CL_DEVICE_VERSION>(),
             (type == CL_DEVICE_TYPE_GPU)
                 ? "GPU"
                 : ((type == CL_DEVICE_TYPE_CPU)
                        ? "CPU"
                        : ((type == CL_DEVICE_TYPE_ACCELERATOR) ? "ACCELERATOR"
                                                                : "OTHER")));
      if (d.getInfo<CL_DEVICE_IMAGE_SUPPORT>() == CL_FALSE) {
        LWARN("tpm::gpu", "OpenCL device({:X}) does not support images",
              device_hash);
        continue;
      }
      cl::DeviceCommandQueue command_queue(*context, d);
      devices[device_hash] = (DeviceData){.command_queue = command_queue,
                                          .image = img,
                                          .context = context,
                                          .program = program};
    }
  }
  return true;
}

std::optional<tpm::Image> tpm::gpu::GPULoadBalancer::execute(const Scene::Instance &scene) {
  return std::optional<tpm::Image>();
}
bool tpm::gpu::GPULoadBalancer::terminate() { return true; }
