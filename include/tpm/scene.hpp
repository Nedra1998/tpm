#ifndef TPM_SCENE_HPP_
#define TPM_SCENE_HPP_

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include "linalg.hpp"
#include "spline.hpp"

namespace tpm {
class Camera {
public:
  struct Instance {
    float fov;
    fVector3 eye, center, up;
  };

  inline Camera::Instance instance(const float &t, const std::size_t &) const {
    return Instance{fov[t], eye[t], center[t], up[t]};
  }

  Spline<float> fov;
  Spline<fVector3> eye, center, up;
};
class Film {
public:
  struct Instance {
    std::string filepath;
    std::pair<std::size_t, std::size_t> resolution;
  };

  inline Film::Instance instance(const float &t, const std::size_t &id) const {
    return Instance{fmt::format(filepath, t, id), resolution};
  }

  std::string filepath;
  std::pair<std::size_t, std::size_t> resolution;
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

inline std::ostream &operator<<(std::ostream &out,
                                const Camera::Instance &rhs) {
  return out << fmt::format("{{fov={}, eye={}, center={}, up={}}}", rhs.fov,
                            rhs.eye, rhs.center, rhs.up);
}
inline std::ostream &operator<<(std::ostream &out, const Camera &rhs) {
  return out << fmt::format("{{fov={}, eye={}, center={}, up={}}}", rhs.fov,
                            rhs.eye, rhs.center, rhs.up);
}
inline std::ostream &operator<<(std::ostream &out, const Film::Instance &rhs) {
  return out << fmt::format("{{filepath={}, resolution={}}}", rhs.filepath,
                            rhs.resolution);
}
inline std::ostream &operator<<(std::ostream &out, const Film &rhs) {
  return out << fmt::format("{{filepath={}, resolution={}}}", rhs.filepath,
                            rhs.resolution);
}
inline std::ostream &operator<<(std::ostream &out, const Scene::Instance &rhs) {
  return out << fmt::format("{{camera={}, film={}, time={}}}", rhs.camera,
                            rhs.film, rhs.time);
}
inline std::ostream &operator<<(std::ostream &out, const Scene &rhs) {
  return out << fmt::format("{{camera={}, film={}, time_range={}, fps={}}}",
                            rhs.camera, rhs.film, rhs.time_range, rhs.fps);
}
} // namespace tpm

#endif /* end of include guard: TPM_SCENE_HPP_ */
