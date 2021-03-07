#include "tpm.hpp"

#include "image.hpp"
#include "linalg.hpp"
#include "logging.hpp"
#include "scene.hpp"
#include "spline.hpp"
#include "version.hpp"

#include "gpu.hpp"

#include <fmt/format.h>

bool tpm::render(const Scene &scene) {
  bool success = true;
  LINFO("tpm", "TPM v{}", version::semver);
  LTRACE("tpm", "tpm::render({})", scene);

  gpu::GPULoadBalancer balancer("kernel void testing(write_only image2d_t output) { for (int i = 0; i < 500; ++i) write_imagef(output, (int2)(i, 10), (float4)(1.0, 0.0, 1.0, 1.0)); }");
  success &= balancer.initialize();

  std::size_t frame_id = 0;
  for (float t = scene.time_range.first; t <= scene.time_range.second;
       t += 1.0f / scene.fps) {
    success &= render_frame(scene.instance(t, frame_id));
    frame_id++;
  }

  success &= balancer.terminate();
  return success;
}
bool tpm::render_frame(const Scene::Instance &scene) {
  LTRACE("tpm", "tpm::render_frame({})", scene);

  Image img(scene.film.resolution.first, scene.film.resolution.second);

  return write_image(scene.film.filepath, img);
}
