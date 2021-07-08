#include "image.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#include "log.hpp"
#include "prof.hpp"
#include "stb_image_write.h"

tpm::image::Tile::Tile(const std::size_t &width, const std::size_t &height,
                       const std::size_t &xoff, const std::size_t &yoff,
                       const std::uint8_t &buffers)
    : width(width), height(height), xoff(xoff), yoff(yoff), buffers(buffers),
      color(), albedo(), normal(), depth() {
  PFUNC(width, height, xoff, yoff, buffers);

  if (buffers & Buffer::COLOR) {
    color = std::vector<float>(width * height * 3);
    std::fill(color.begin(), color.end(), 0.0f);
  }

  if (buffers & Buffer::ALBEDO) {
    albedo = std::vector<float>(width * height * 3);
    std::fill(albedo.begin(), albedo.end(), 0.0f);
  }

  if (buffers & Buffer::NORMAL) {
    normal = std::vector<float>(width * height * 3);
    std::fill(normal.begin(), normal.end(), 0.0f);
  }

  if (buffers & Buffer::DEPTH) {
    depth = std::vector<float>(width * height * 1);
    std::fill(depth.begin(), depth.end(), 0.0f);
  }
}

tpm::image::Tile::~Tile() {
  PFUNC()
  std::vector<float>().swap(color);
  std::vector<float>().swap(albedo);
  std::vector<float>().swap(normal);
  std::vector<float>().swap(depth);
}

tpm::image::Image::Image(const std::size_t &width, const std::size_t &height,
                         const std::size_t &tile_size,
                         const std::uint8_t &buffers)
    : width(width), height(height), tile_size(tile_size), buffers(buffers),
      color(nullptr), albedo(nullptr), normal(nullptr), depth(nullptr) {
  PFUNC(width, height, buffers);

  if (buffers & Buffer::COLOR) {
    color =
        static_cast<float *>(std::malloc(width * height * 3 * sizeof(float)));
    std::fill(color, color + (width * height * 3), 0.0f);
  }

  if (buffers & Buffer::ALBEDO) {
    albedo =
        static_cast<float *>(std::malloc(width * height * 3 * sizeof(float)));
    std::fill(albedo, albedo + (width * height * 3), 0.0f);
  }

  if (buffers & Buffer::NORMAL) {
    normal =
        static_cast<float *>(std::malloc(width * height * 3 * sizeof(float)));
    std::fill(normal, normal + (width * height * 3), 0.0f);
  }

  if (buffers & Buffer::DEPTH) {
    depth =
        static_cast<float *>(std::malloc(width * height * 1 * sizeof(float)));
    std::fill(depth, depth + (width * height * 1), 0.0f);
  }
}

tpm::image::Image::~Image() {
  PFUNC()
  if (color != nullptr)
    free(color);
  if (albedo != nullptr)
    free(albedo);
  if (normal != nullptr)
    free(normal);
  if (depth != nullptr)
    free(depth);
}

const float *tpm::image::Image::get_buffer(const Buffer &buffer) const {
  if (buffer == Buffer::COLOR)
    return color;
  else if (buffer == Buffer::ALBEDO)
    return albedo;
  else if (buffer == Buffer::NORMAL)
    return normal;
  else if (buffer == Buffer::DEPTH)
    return depth;
  return nullptr;
}

tpm::image::Tile tpm::image::Image::get_tile(const std::size_t &i) const {
  PFUNC(i);
  std::size_t tiles_wide =
      ((width / tile_size) + (width % tile_size != 0 ? 1 : 0));
  std::size_t tiles_tall =
      ((height / tile_size) + (height % tile_size != 0 ? 1 : 0));
  std::size_t x = i % tiles_wide, y = i / tiles_wide;
  return Tile(x == tiles_wide - 1 ? tile_size - (tiles_wide * tile_size - width)
                                  : tile_size,
              y == tiles_tall - 1
                  ? tile_size - (tiles_tall * tile_size - height)
                  : tile_size,
              x * tile_size, y * tile_size, buffers);
}

std::vector<tpm::image::Tile> tpm::image::Image::get_tiles() const {
  PFUNC();
  std::vector<Tile> tiles;
  tiles.reserve(tile_count());
  std::size_t tiles_wide =
      ((width / tile_size) + (width % tile_size != 0 ? 1 : 0));
  std::size_t tiles_tall =
      ((height / tile_size) + (height % tile_size != 0 ? 1 : 0));
  for (std::size_t i = 0; i < tile_count(); ++i) {
    std::size_t x = i % tiles_wide, y = i / tiles_wide;
    tiles.emplace_back(
        x == tiles_wide - 1 ? tile_size - (tiles_wide * tile_size - width)
                            : tile_size,
        y == tiles_tall - 1 ? tile_size - (tiles_tall * tile_size - height)
                            : tile_size,
        x * tile_size, y * tile_size, buffers);
  }
  return tiles;
}

void tpm::image::Image::merge_tile(const Tile &tile) {
  PFUNC(&tile);
  for (std::size_t y = 0; y < tile.height; ++y) {
    std::size_t t = y * tile.width;
    std::size_t i = (tile.yoff + y) * width + tile.xoff;
    if (!tile.color.empty())
      std::memcpy(color + (i * 3), tile.color.data() + (t * 3),
                  tile.width * 3 * sizeof(float));
    if (!tile.albedo.empty())
      std::memcpy(albedo + (i * 3), tile.albedo.data() + (t * 3),
                  tile.width * 3 * sizeof(float));
    if (!tile.normal.empty())
      std::memcpy(normal + (i * 3), tile.normal.data() + (t * 3),
                  tile.width * 3 * sizeof(float));
    if (!tile.depth.empty())
      std::memcpy(depth + (i), tile.depth.data() + (t),
                  tile.width * sizeof(float));
  }
}

std::size_t tpm::image::Image::tile_count() const {
  return ((width / tile_size) + (width % tile_size != 0 ? 1 : 0)) *
         ((height / tile_size) + (height % tile_size != 0 ? 1 : 0));
}

void tpm::image::convert_buffer(const std::size_t &size, const float *in,
                                std::uint8_t *out) {
  PFUNC(size);
  for (std::size_t i = 0; i < size; ++i)
    out[i] = static_cast<std::uint8_t>(in[i] * 255.0f);
}

bool tpm::image::write(const std::string &file_path, const Image &img,
                       const std::uint8_t &buffers) {
  PFUNC(file_path, &img, buffers);
  std::filesystem::path path(file_path);
  if ((buffers & img.buffers) == 0) {
    LWARN("Image does not contain any matching buffer types {} != {}. No "
          "images will be written",
          img.buffers, buffers);
    return false;
  }

  bool ret = true;

  if (ret && buffers & img.buffers & Buffer::COLOR)
    ret &= write_buffer(std::filesystem::path(fmt::format(
                            file_path, fmt::arg("buffer", "color"))),
                        img, Buffer::COLOR);

  if (ret && buffers & img.buffers & Buffer::ALBEDO)
    ret &= write_buffer(std::filesystem::path(fmt::format(
                            file_path, fmt::arg("buffer", "albedo"))),
                        img, Buffer::ALBEDO);

  if (ret && buffers & img.buffers & Buffer::NORMAL)
    ret &= write_buffer(std::filesystem::path(fmt::format(
                            file_path, fmt::arg("buffer", "normal"))),
                        img, Buffer::NORMAL);

  if (ret && buffers & img.buffers & Buffer::DEPTH)
    ret &= write_buffer(std::filesystem::path(fmt::format(
                            file_path, fmt::arg("buffer", "depth"))),
                        img, Buffer::DEPTH);

  return ret;
}

bool tpm::image::write_buffer(const std::filesystem::path &path,
                              const Image &img, const Buffer &buffer) {
  PFUNC(path.string(), &img, buffer);
  std::uint8_t depth = buffer & Buffer::DEPTH ? 1 : 3;
  if (path.extension() == ".png") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.width * img.height * depth * sizeof(std::uint8_t)));
    convert_buffer(img.width * img.height * depth, img.get_buffer(buffer),
                   data);
    bool ret =
        (stbi_write_png(path.c_str(), static_cast<int>(img.width),
                        static_cast<int>(img.height), static_cast<int>(depth),
                        data, static_cast<int>(img.width * depth)) != 0);
    free(data);
    return ret;
  } else if (path.extension() == ".bmp") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.width * img.height * depth * sizeof(std::uint8_t)));
    convert_buffer(img.width * img.height * depth, img.get_buffer(buffer),
                   data);
    bool ret = (stbi_write_bmp(path.c_str(), static_cast<int>(img.width),
                               static_cast<int>(img.height),
                               static_cast<int>(depth), data) != 0);
    free(data);
    return ret;
  } else if (path.extension() == ".tga") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.width * img.height * depth * sizeof(std::uint8_t)));
    convert_buffer(img.width * img.height * depth, img.get_buffer(buffer),
                   data);
    bool ret = (stbi_write_tga(path.c_str(), static_cast<int>(img.width),
                               static_cast<int>(img.height),
                               static_cast<int>(depth), data) != 0);
    free(data);
    return ret;
  } else if (path.extension() == ".jpg") {
    std::uint8_t *data = static_cast<std::uint8_t *>(
        malloc(img.width * img.height * depth * sizeof(std::uint8_t)));
    convert_buffer(img.width * img.height * depth, img.get_buffer(buffer),
                   data);
    bool ret = (stbi_write_jpg(path.c_str(), static_cast<int>(img.width),
                               static_cast<int>(img.height),
                               static_cast<int>(depth), data, 100) != 0);
    free(data);
    return ret;
  } else if (path.extension() == ".hdr") {
    return stbi_write_hdr(path.c_str(), static_cast<int>(img.width),
                          static_cast<int>(img.height), static_cast<int>(depth),
                          img.get_buffer(buffer)) != 0;
  } else {
    LWARN("Unrecognized image extension \"{}\"", path.extension().c_str());
    return false;
  }
}