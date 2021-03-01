#include "tpm.hpp"

#include "image.hpp"
#include "linalg.hpp"
#include "logging.hpp"
#include "scene.hpp"
#include "spline.hpp"
#include "version.hpp"

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

  Image img(scene.film.resolution.first, scene.film.resolution.second);

  return write_image(scene.film.filepath, img);
}
