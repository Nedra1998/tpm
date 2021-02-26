#ifndef TPM_SCENE_HPP_
#define TPM_SCENE_HPP_

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "glm_ops.hpp"
#include "spline.hpp"

namespace tpm {
class Camera {
public:
  struct Instance {
    float fov;
    glm::vec3 eye, center, up;
  };

  inline Camera::Instance instance(const float &t, const std::size_t &) const {
    return Instance{fov[t], eye[t], center[t], up[t]};
  }

  Spline<float> fov;
  Spline<glm::vec3> eye, center, up;
};
class Film {
public:
  struct Instance {
    std::string filepath;
    glm::uvec2 resolution;
  };

  inline Film::Instance instance(const float &t, const std::size_t &id) const {
    return Instance{fmt::format(filepath, t, id), resolution};
  }

  std::string filepath;
  glm::uvec2 resolution;
};
class Scene {
public:
  struct Instance {
    Camera::Instance camera;
    Film::Instance film;

    float time;
  };

  inline Scene::Instance instance(const float &t, const std::size_t &id) const {
    return Instance{camera.instance(t, id), film.instance(t, id), t};
  }

  Camera camera;
  Film film;
  std::pair<float, float> time_range;
  float fps;
};

template <typename T, typename U>
inline std::ostream &operator<<(std::ostream &out, const Spline<T, U> &spline) {
  return out << fmt::format("{}", spline.keyframes);
}
inline std::ostream &operator<<(std::ostream &out,
                                const Camera::Instance &rhs) {
  return out << fmt::format(
             "Camera::Instance{{fov={}, eye={}, center={}, up={}}}", rhs.fov,
             rhs.eye, rhs.center, rhs.up);
}
inline std::ostream &operator<<(std::ostream &out, const Camera &rhs) {
  return out << fmt::format(
             "Camera::Instance{{fov={}, eye={}, center={}, up={}}}", rhs.fov,
             rhs.eye, rhs.center, rhs.up);
}
inline std::ostream &operator<<(std::ostream &out, const Film::Instance &rhs) {
  return out << fmt::format("Film{{filepath={}, resolution={}}}", rhs.filepath,
                            rhs.resolution);
}
inline std::ostream &operator<<(std::ostream &out, const Film &rhs) {
  return out << fmt::format("Film{{filepath={}, resolution={}}}", rhs.filepath,
                            rhs.resolution);
}
inline std::ostream &operator<<(std::ostream &out, const Scene::Instance &rhs) {
  return out << fmt::format("Scene::Instance{{camera={}, film={}, time={}}}",
                            rhs.camera, rhs.film, rhs.time);
}
inline std::ostream &operator<<(std::ostream &out, const Scene &rhs) {
  return out << fmt::format(
             "Scene{{camera={}, film={}, time_range={}, fps={}}}", rhs.camera,
             rhs.film, rhs.time_range, rhs.fps);
}
} // namespace tpm

#endif /* end of include guard: TPM_SCENE_HPP_ */
