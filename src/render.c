#include "render.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "cpu.h"
#include "image.h"
#include "log.h"
#include "stretchy_buffer.h"

#include "gpu.h"

/* void render_image(const RenderImageArgs args) { */
/*   float *film = (float *)malloc(sizeof(float) * 3 * args.resolution.x * */
/*                                 args.resolution.y); */
/*   float filmz = args.resolution.x / (2.0f * tan(args.camera.fov / 2.0f)); */
/*   ldebug("X: %f, Y: %f", args.resolution.x, args.resolution.y); */
/*   ldebug("FOV: %f -> %f", args.camera.fov, filmz); */
/*   float view[16] = {0.0f}; */
/*   look_at(args.camera.eye, args.camera.center, args.camera.up, view); */
/*   ldebug("EYE: %f,%f,%f CENTER: %f,%f,%f UP: %f,%f,%f", args.camera.eye[0],
 */
/*          args.camera.eye[1], args.camera.eye[2], args.camera.center[0], */
/*          args.camera.center[1], args.camera.center[2], args.camera.up[0], */
/*          args.camera.up[1], args.camera.up[2]); */
/*   ldebug("[[%f,%f,%f,%f],[%f,%f,%f,%f],[%f,%f,%f,%f],[%f,%f,%f,%f]]",
 * view[0], */
/*          view[1], view[2], view[3], view[4], view[5], view[6], view[7],
 * view[8], */
/*          view[9], view[10], view[11], view[12], view[13], view[14],
 * view[15]); */
/*   mat_inverse(view, view); */
/*   ldebug("[[%f,%f,%f,%f],[%f,%f,%f,%f],[%f,%f,%f,%f],[%f,%f,%f,%f]]",
 * view[0], */
/*          view[1], view[2], view[3], view[4], view[5], view[6], view[7],
 * view[8], */
/*          view[9], view[10], view[11], view[12], view[13], view[14],
 * view[15]); */
/*   float origin[3] = {0.0f, 0.0f, 0.0f}; */
/*   mat_mult_point(view, origin, origin); */
/*  */
/*   for (int i = 0; i < args.resolution.x * args.resolution.y; ++i) { */
/*     int x = i % args.resolution.x; */
/*     int y = i / args.resolution.x; */
/*     float *pixel = &film[i * 3]; */
/*     pixel[0] = 0.0f; */
/*     pixel[1] = 0.0f; */
/*     pixel[2] = 0.0f; */
/*     for (int j = 0; j < args.spp; ++j) { */
/*       float direction[3] = {x - args.resolution.x / 2.0f, */
/*                             y - args.resolution.y / 2.0f, filmz}; */
/*       mat_mult_dir(view, direction, direction); */
/*       vec_normalize(direction, direction); */
/*       path_trace(origin, direction, pixel); */
/*       ldebug("Ray: %f,%f,%f -> %f,%f,%f : %f,%f,%f", origin[0], origin[1], */
/*              origin[2], direction[0], direction[1], direction[2], pixel[0],
 */
/*              pixel[1], pixel[2]); */
/*     } */
/*     pixel[0] /= args.spp; */
/*     pixel[1] /= args.spp; */
/*     pixel[2] /= args.spp; */
/*   } */
/*  */
/*   write_image(args.filename, args.resolution.x, args.resolution.y, film); */
/*   free(film); */
/* } */

void render_image(const RenderImageArgs args) {
  float *film = (float *)malloc(sizeof(float) * 3 * args.resolution.x *
                                args.resolution.y);
  float view[16] = {0.0f};
  look_at(args.camera.eye, args.camera.center, args.camera.up, view);
  mat_inverse(view, view);

  cl_int err;
  cl_uint num_platforms;
  cl_platform_id platform_id;
  cl_context context = NULL;

  err = clGetPlatformIDs(1, &platform_id, &num_platforms);
  if (err != CL_SUCCESS || num_platforms <= 0) {
    lcrit("Failed to find any OpenCL platforms");
    return;
  }

  cl_context_properties context_properties[] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)platform_id, 0};
  context = clCreateContextFromType(context_properties, CL_DEVICE_TYPE_GPU,
                                    NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    lcrit("Failed to create an OpenCL GPU context");
    return;
  }

  size_t device_buffer_size = -1;
  cl_device_id *devices;
  cl_command_queue command_queue = NULL;

  // First get the size of the devices buffer
  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL,
                         &device_buffer_size);
  if (err != CL_SUCCESS) {

    lcrit("Failed call to clGetContextInfo(...,GL_CONTEXT_DEVICES,...)");
    return;
  }

  if (device_buffer_size <= 0) {
    lcrit("No devices available.");
    return;
  }

  // Allocate memory for the devices buffer
  devices = (cl_device_id *)malloc(device_buffer_size * sizeof(cl_device_id));
  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, device_buffer_size,
                         devices, NULL);
  if (err != CL_SUCCESS) {
    free(devices);
    lcrit("Failed to get device IDs");
    return;
  }

  // In this example, we just choose the first available device.  In a
  // real program, you would likely use all available devices or choose
  // the highest performance device based on OpenCL device queries
  command_queue = clCreateCommandQueue(context, devices[0], 0, NULL);
  if (command_queue == NULL) {
    free(devices);
    lcrit("Failed to create command_queue for device 0");
    return;
  }

  cl_device_id device = devices[0];
  free(devices);

  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&gpu_kernel, NULL, NULL);
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  cl_kernel kernel = clCreateKernel(program, "renderImage", NULL);

  cl_image_format format = {.image_channel_order = CL_RGBA,
                            .image_channel_data_type = CL_FLOAT};
  cl_image_desc desc = {.image_type = CL_MEM_OBJECT_IMAGE2D,
                        .image_width = args.resolution.x,
                        .image_height = args.resolution.y,
                        .image_depth = 3};

  cl_mem image =
      clCreateImage(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, &format,
                    &desc, &film[0], &err);
  if (err != CL_SUCCESS) {
    lerr("Failed to create OpenCL image2d_t: %d", err);
  }

  cl_mem cl_view =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * 16, view, &err);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &cl_view);

  size_t global_work_size[1] = {1};
  size_t local_work_size[1] = {1};

  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size,
                               local_work_size, 0, NULL, NULL);

  size_t origin[] = {0, 0};
  size_t region[] = {args.resolution.x, args.resolution.y, 1};

  err = clEnqueueReadImage(command_queue, image, CL_TRUE, &origin, &region, 0,
                           0, film, 0, NULL, NULL);

  write_image(args.filename, args.resolution.x, args.resolution.y, film);
  clReleaseCommandQueue(command_queue);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseContext(context);

  free(film);
}
