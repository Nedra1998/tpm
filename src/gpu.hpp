#ifndef TPM_GPU_HPP_
#define TPM_GPU_HPP_

#include <unordered_map>
#if __has_include(<CL/opencl.hpp>)
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include <CL/opencl.hpp>
#else
#error "OpenCL C++ API is required for TPM GPU support"
#endif

#include "compiler.hpp"
#include "image.hpp"
#include "scene.hpp"

namespace tpm::gpu {
class LoadBalancer {
public:
  virtual ~LoadBalancer() {}

  virtual bool initialize() = 0;
  virtual std::optional<tpm::Image> execute(const Scene::Instance &scene) = 0;
  virtual bool terminate() = 0;
};
class GPULoadBalancer : public LoadBalancer {
public:
  GPULoadBalancer(const std::string &kernel) : kernel(kernel) {}

  bool initialize() TPM_OVERRIDE;
  std::optional<tpm::Image> execute(const Scene::Instance &scene) TPM_OVERRIDE;
  bool terminate() TPM_OVERRIDE;

private:
  struct DeviceData {
    cl::DeviceCommandQueue command_queue;
    std::shared_ptr<cl::Image2D> image;
    std::shared_ptr<cl::Context> context;
    std::shared_ptr<cl::Program> program;
  };

  std::string kernel;
  std::unordered_map<std::size_t, DeviceData> devices;
};
} // namespace tpm::gpu

#endif /* end of include guard: TPM_GPU_HPP_ */
