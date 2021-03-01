#include "image.hpp"
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "logging.hpp"

std::vector<std::uint8_t>
tpm::image::convert_to_bits(const std::vector<float> &buffer) {
  std::vector<std::uint8_t> out(buffer.size());
  std::transform(buffer.begin(), buffer.end(), out.begin(),
                 [](const float &v) -> std::uint8_t {
                   return static_cast<std::uint8_t>(std::clamp(v, 0.0f, 1.0f) *
                                                    0xFF);
                 });
  return out;
}

bool tpm::image::write_bpm(const std::string &file, const Image &img) {
  std::vector<std::uint8_t> bits = convert_to_bits(img.buffer);
  if (stbi_write_bmp(file.c_str(), static_cast<int>(img.width),
                     static_cast<int>(img.height), 3, bits.data()) == 0) {
    LERR("tpm::image", "Failed to write BMP \"{}\" image file", file);
    return false;
  }
  LINFO("tpm::image", "Wrote BMP image \"{}\"", file);
  return true;
}
bool tpm::image::write_hdr(const std::string &file, const Image &img) {
  if (stbi_write_hdr(file.c_str(), static_cast<int>(img.width),
                     static_cast<int>(img.height), 3, img.buffer.data()) == 0) {
    LERR("tpm::image", "Failed to write HDR \"{}\" image file", file);
    return false;
  }
  LINFO("tpm::image", "Wrote HDR image \"{}\"", file);
  return true;
}
bool tpm::image::write_jpg(const std::string &file, const Image &img) {
  std::vector<std::uint8_t> bits = convert_to_bits(img.buffer);
  if (stbi_write_jpg(file.c_str(), static_cast<int>(img.width),
                     static_cast<int>(img.height), 3, bits.data(), 100) == 0) {
    LERR("tpm::image", "Failed to write JPG \"{}\" image file", file);
    return false;
  }
  LINFO("tpm::image", "Wrote JPG image \"{}\"", file);
  return true;
}
bool tpm::image::write_tga(const std::string &file, const Image &img) {
  std::vector<std::uint8_t> bits = convert_to_bits(img.buffer);
  if (stbi_write_tga(file.c_str(), static_cast<int>(img.width),
                     static_cast<int>(img.height), 3, bits.data()) == 0) {
    LERR("tpm::image", "Failed to write TGA \"{}\" image file", file);
    return false;
  }
  LINFO("tpm::image", "Wrote TGA image \"{}\"", file);
  return true;
}
bool tpm::image::write_png(const std::string &file, const Image &img) {
  std::vector<std::uint8_t> bits = convert_to_bits(img.buffer);
  if (stbi_write_png(file.c_str(), static_cast<int>(img.width),
                     static_cast<int>(img.height), 3, bits.data(),
                     static_cast<int>(sizeof(std::uint8_t) * img.width * 3)) ==
      0) {
    LERR("tpm::image", "Failed to write PNG \"{}\" image file", file);
    return false;
  }
  LINFO("tpm::image", "Wrote PNG image \"{}\"", file);
  return true;
}

bool tpm::write_image(const std::string &file, const Image &img) {
  std::filesystem::path filepath(file);
  std::filesystem::path dirpath = std::filesystem::absolute(filepath).parent_path();
  if(!std::filesystem::exists(dirpath)) {
    if (!std::filesystem::create_directories(dirpath)) {
      LERR("tpm::image", "Failed to create output directories \"{}\"", dirpath);
    }else {
      LINFO("tpm::image", "Created image output path \"{}\"", dirpath);
    }
  }

  if (file.ends_with(".png")) {
    return image::write_png(file, img);
  } else if (file.ends_with(".bmp")) {
    return image::write_bpm(file, img);
  } else if (file.ends_with(".tga")) {
    return image::write_tga(file, img);
  } else if (file.ends_with(".jpg") || file.ends_with(".jpeg")) {
    return image::write_jpg(file, img);
  } else if (file.ends_with(".hdr")) {
    return image::write_hdr(file, img);
  } else {
    LWARN("tpm::image", "Unrecognized image extension \"{}\"", file);
    return false;
  }
}
