#ifndef SYSINFO_H_FEOR0AXW
#define SYSINFO_H_FEOR0AXW

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

void log_sysinfo();
void log_platforms();
void log_devices(cl_platform_id platform);

#endif /* end of include guard: SYSINFO_H_FEOR0AXW */
