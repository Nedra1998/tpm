#include "tpm.hpp"

#include "logging.hpp"
#include "spline.hpp"
#include "version.hpp"
#include "scene.hpp"
#include "linalg.hpp"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <fmt/format.h>

bool tpm::render(const Scene &scene) {
  LINFO("tpm", "TPM v{}", version::semver);
  LTRACE("tpm", "tpm::render({})", scene);
  
  bool success = true;
  std::size_t frame_id = 0;
  for (float t = scene.time_range.first; t <= scene.time_range.second;
       t += 1.0f / scene.fps) {
    success &= render_frame(scene.instance(t, frame_id));
    frame_id++;
  }
  return success;
}
bool tpm::render_frame(const Scene::Instance &scene) {
  LTRACE("tpm", "tpm::render_frame({})", scene);
  return true;
}
