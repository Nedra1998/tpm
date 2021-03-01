#ifndef TPM_IMAGE_HPP_
#define TPM_IMAGE_HPP_

#include <cstdlib>
#include <string>
#include <vector>

namespace tpm {
struct Image {
  Image(std::size_t width, std::size_t height)
      : width(width), height(height) {
    buffer.resize(width * height * 3);
  }

  std::size_t width, height;
  std::vector<float> buffer;
};

namespace image {
  std::vector<std::uint8_t> convert_to_bits(const std::vector<float>& buffer);
  bool write_bpm(const std::string& file, const Image& img);
  bool write_hdr(const std::string& file, const Image& img);
  bool write_jpg(const std::string& file, const Image& img);
  bool write_png(const std::string& file, const Image& img);
  bool write_tga(const std::string& file, const Image& img);
} // namespace image
bool write_image(const std::string& file, const Image& img);

} // namespace tpm

#endif // TPM_IMAGE_HPP_
