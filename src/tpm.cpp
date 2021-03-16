#include "tpm.hpp"

#include "image.hpp"
#include "linalg.hpp"
#include "logging.hpp"
#include "scene.hpp"
#include "spline.hpp"
#include "version.hpp"

#include "gpu.hpp"
#include "host.hpp"
#include "orchestrator.hpp"

#include <fmt/format.h>

const std::string kernel = "void kernel tpm(global float *image, const uint2 size) { for (int i = 0; i < size.x * size.y; ++i) { image[3 * i] = 1.0f; image[3 * i + 1] = 0.0f; image[3 * i + 2] = 1.0f; } } ";

bool tpm::render(const Scene &scene) {
  bool success = true;
  LINFO("tpm", "TPM v{}", version::semver);
  LTRACE("tpm", "tpm::render({})", scene);

  Orchestrator orchestrator;
  orchestrator.push_back(std::make_shared<host::HostLocal>());
  orchestrator.initialize();
  orchestrator.compile(kernel);

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
