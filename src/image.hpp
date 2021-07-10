#ifndef TPM_IMAGE_HPP
#define TPM_IMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

namespace tpm::image {

enum Buffer {
  COLOR = 1 << 0,
  ALBEDO = 1 << 1,
  NORMAL = 1 << 2,
  DEPTH = 1 << 3,
  ALL = COLOR | ALBEDO | NORMAL | DEPTH
};

class Tile {
public:
  Tile(const std::size_t &width, const std::size_t &height,
       const std::size_t &xoff, const std::size_t &yoff,
       const std::uint8_t &buffers = Buffer::ALL);
  ~Tile();

  inline void set(const Buffer &buffer, const std::size_t &x,
                  const std::size_t &y, const float &r = 0.0f,
                  const float &g = 0.0f, const float &b = 0.0f) {
    if (x >= width || y >= height)
      return;
    std::size_t i = (y * width + x) * 3;
    if (buffer == Buffer::COLOR && !color.empty()) {
      color[i + 0] = r;
      color[i + 1] = g;
      color[i + 2] = b;
    } else if (buffer == Buffer::ALBEDO && !albedo.empty()) {
      albedo[i + 0] = r;
      albedo[i + 1] = g;
      albedo[i + 2] = b;
    } else if (buffer == Buffer::NORMAL && !normal.empty()) {
      normal[i + 0] = r;
      normal[i + 1] = g;
      normal[i + 2] = b;
    } else if (buffer == Buffer::DEPTH && !depth.empty()) {
      i = (y * width + x);
      depth[i] = r;
    }
  }

  std::size_t width, height, xoff, yoff;
  std::uint8_t buffers;

private:
  std::vector<float> color, albedo, normal, depth;

  friend class Image;
};

class Image {
public:
  Image(const std::size_t &width, const std::size_t &height,
        const std::size_t &tile_size,
        const std::uint8_t &buffers = Buffer::ALL);
  ~Image();

  const float *get_buffer(const Buffer &buffer) const;

  Tile get_tile(const std::size_t &i) const;
  std::vector<Tile> get_tiles(
      const std::size_t &begin = 0,
      const std::size_t &end = std::numeric_limits<std::size_t>().max()) const;
  void merge_tile(const Tile &tile);
  std::size_t tile_count() const;

  inline std::size_t size() const { return width * height; }
  inline float *get_color() { return color; }

  std::size_t width, height, tile_size;
  std::uint8_t buffers;

private:
  float *color, *albedo, *normal, *depth;
};

void convert_buffer(const std::size_t &size, const float *in,
                    std::uint8_t *out);

bool write(const std::string &file_path, const Image &img,
           const std::uint8_t &buffers = Buffer::ALL);
bool write_buffer(const std::filesystem::path &path, const Image &img,
                  const Buffer &buffer);
} // namespace tpm::image

#endif /* end of include guard: TPM_IMAGE_HPP */
