#ifndef TPM_HPP_
#define TPM_HPP_

#include "scene.hpp"

namespace tpm {
bool render(const Scene &scene);
bool render_frame(const Scene::Instance &scene);
} // namespace tpm

#endif /* end of include guard: TPM_HPP_ */
