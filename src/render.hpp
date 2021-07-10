#ifndef RENDER_HPP_ZDML5IXE
#define RENDER_HPP_ZDML5IXE

#include <CL/sycl.hpp>
#include <hipSYCL/sycl/queue.hpp>

#include "image.hpp"

namespace tpm {
bool render(cl::sycl::queue &q, image::Image &img);
bool render_tile_batch(cl::sycl::queue &q, image::Image &img,
                       const std::size_t &begin, const std::size_t &batch_size);
} // namespace tpm

#endif /* end of include guard: RENDER_HPP_ZDML5IXE */
