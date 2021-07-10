#include "image.hpp"

#include <CL/sycl.hpp>
#include <hipSYCL/sycl/device.hpp>
#include <hipSYCL/sycl/handler.hpp>
#include <hipSYCL/sycl/info/device.hpp>
#include <hipSYCL/sycl/info/platform.hpp>
#include <hipSYCL/sycl/platform.hpp>
#include <hipSYCL/sycl/queue.hpp>

#include "prof.hpp"
#include "render.hpp"

bool tpm::render(cl::sycl::queue &q, image::Image &img) {
  PFUNC(&q, &img);
  std::size_t tile_batch_size = 1024 * 8;
  bool ret = true;
  for (std::size_t i = 0; i < img.tile_count(); i += tile_batch_size)
    ret &= render_tile_batch(q, img, i, tile_batch_size);
  return ret;
}

bool tpm::render_tile_batch(cl::sycl::queue &q, image::Image &img,
                            const std::size_t &begin,
                            const std::size_t &batch_size) {
  PFUNC(&q, &img, begin, batch_size);

  PBEGIN("InitializeTiles");
  std::vector<tpm::image::Tile> tiles =
      img.get_tiles(begin, begin + batch_size);
  cl::sycl::range<1> work_items{tiles.size()};
  PEND("InitializeTiles");

  {
    PSCOPE("CommandGroup", tiles.size());
    cl::sycl::buffer<tpm::image::Tile> tile_buffer(tiles.data(), img.size());
    q.submit([&](cl::sycl::handler &cgh) {
      auto access_tiles =
          tile_buffer.get_access<cl::sycl::access::mode::read_write>(cgh);
      cgh.parallel_for<class vector_add>(
          work_items, [=](cl::sycl::item<1> item) {
            PSCOPE("Kernel", item.get_linear_id());

            float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

            tpm::image::Tile *tile = &access_tiles[item.get_linear_id()];
            for (std::size_t x = 0; x < tile->width; ++x)
              for (std::size_t y = 0; y < tile->height; ++y)
                tile->set(tpm::image::COLOR, x, y, r, g, b);
          });
    });
  }
  PBEGIN("MergeTiles");
  for (auto &tile : tiles)
    img.merge_tile(tile);
  PEND("MergeTiles");

  return true;
}
