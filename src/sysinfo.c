#include "sysinfo.h"

#include <stdint.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "compiler.h"
#include "log.h"
#include "version.h"

void log_sysinfo() {
  linfo("%s", TPM_VERSION_STRING);
#if TPM_COMPILER_IS_AppleClang
  ldebug("Compiler: AppleClang v%d.%d.%d", TPM_COMPILER_VERSION_MAJOR,
         TPM_COMPILER_VERSION_MINOR, TPM_COMPILER_VERSION_PATCH);
#elif TPM_COMPILER_IS_Clang
  ldebug("Compiler: Clang v%d.%d.%d", TPM_COMPILER_VERSION_MAJOR,
         TPM_COMPILER_VERSION_MINOR, TPM_COMPILER_VERSION_PATCH);
#elif TPM_COMPILER_IS_GNU
  ldebug("Compiler: GNU v%d.%d.%d", TPM_COMPILER_VERSION_MAJOR,
         TPM_COMPILER_VERSION_MINOR, TPM_COMPILER_VERSION_PATCH);
#elif TPM_COMPILER_IS_Intel
  ldebug("Compiler: Intel v%d.%d.%d", TPM_COMPILER_VERSION_MAJOR,
         TPM_COMPILER_VERSION_MINOR, TPM_COMPILER_VERSION_PATCH);
#elif TPM_COMPILER_IS_MSVC
  ldebug("Compiler: MSVC v%d.%d.%d", TPM_COMPILER_VERSION_MAJOR,
         TPM_COMPILER_VERSION_MINOR, TPM_COMPILER_VERSION_PATCH);
#endif
  log_platforms();
}

void log_platforms() {
  cl_platform_id platforms[8];
  cl_uint num_platforms;
  cl_int ret = clGetPlatformIDs(8, platforms, &num_platforms);
  if (ret != CL_SUCCESS) {
    lwarn("Failed to get CL platform ids");
    return;
  }
  for (cl_uint i = 0; i < num_platforms; ++i) {
    char name[64], version[64], profile[32];
    ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 64, name, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_PLATFORM_NAME (%d)", i);
    ret =
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 64, version, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_PLATFORM_VERSION (%d)", i);
    ret =
        clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 32, profile, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_PLATFORM_PROFILE (%d)", i);
    ldebug("Platform: %s : %s : %s", name, version, profile);
    log_devices(platforms[i]);
  }
}

void log_devices(cl_platform_id platform) {
  cl_device_id devices[32];
  cl_uint num_devices;
  cl_int ret =
      clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 32, devices, &num_devices);
  if (ret != CL_SUCCESS) {
    lwarn("Failed to get CL device ids");
    return;
  }
  for (cl_uint i = 0; i < num_devices; ++i) {
    char name[64], version[64], profile[32];
    cl_device_type type;
    cl_uint max_compute_units;
    ret =
        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(type), &type, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_DEVICE_TYPE (%d)", i);
    ret = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS,
                          sizeof(max_compute_units), &max_compute_units, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_DEVICE_MAX_COMPUTE_UNITS (%d)", i);
    ret = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 64, name, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_DEVICE_NAME (%d)", i);
    ret = clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, 64, version, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_DEVICE_VERSION (%d)", i);
    ret = clGetDeviceInfo(devices[i], CL_DEVICE_PROFILE, 32, profile, NULL);
    if (ret != CL_SUCCESS)
      lwarn("Failed to get CL_DEVICE_PROFILE (%d)", i);

    if (type == CL_DEVICE_TYPE_CPU) {
      ldebug("  CPU: %s : %s : %s : %u", name, version,
             profile, max_compute_units);
    } else if (type == CL_DEVICE_TYPE_GPU) {
      ldebug("  GPU: %s : %s : %s : %u", name, version,
             profile, max_compute_units);
    } else if (type == CL_DEVICE_TYPE_ACCELERATOR) {
      ldebug("  ACCELERATOR: %s : %s : %s : %u", name, 
             version, profile, max_compute_units);
    } else {
      ldebug("  OTHER: %s : %s : %s : %u", name, version,
             profile, max_compute_units);
    }
  }
}
