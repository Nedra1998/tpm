#include "image.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "log.h"
#include "stb.h"
#include "stb_image_write.h"

void float_to_8bit(const float *in, int size, uint8_t *out) {
  for (int i = 0; i < size; ++i) {
    out[i] = (uint8_t)(in[i] * 0xFF);
  }
}

bool write_bmp(const char *filename, int width, int height, const float *data) {
  ldebug("Writting BMP \"%s\"", filename);
  uint8_t *bits = (uint8_t *)malloc(width * height * 3);
  float_to_8bit(data, width * height * 3, bits);
  bool res = stbi_write_bmp(filename, width, height, 3, bits) != 0;
  free(bits);
  return res;
}
bool write_png(const char *filename, int width, int height, const float *data) {
  ldebug("Writting PNG \"%s\"", filename);
  uint8_t *bits = (uint8_t *)malloc(width * height * 3);
  float_to_8bit(data, width * height * 3, bits);
  bool res = stbi_write_png(filename, width, height, 3, bits,
                            width * 3 * sizeof(uint8_t)) != 0;
  free(bits);
  return res;
}
bool write_tga(const char *filename, int width, int height, const float *data) {
  ldebug("Writting TGA \"%s\"", filename);
  uint8_t *bits = (uint8_t *)malloc(width * height * 3);
  float_to_8bit(data, width * height * 3, bits);
  bool res = stbi_write_tga(filename, width, height, 3, bits) != 0;
  free(bits);
  return res;
}
bool write_jpg(const char *filename, int width, int height, const float *data) {
  ldebug("Writting JPEG \"%s\"", filename);
  uint8_t *bits = (uint8_t *)malloc(width * height * 3);
  float_to_8bit(data, width * height * 3, bits);
  bool res = stbi_write_jpg(filename, width, height, 3, bits, 100) != 0;
  free(bits);
  return res;
}
bool write_hdr(const char *filename, int width, int height, const float *data) {
  ldebug("Writting HDR \"%s\"", filename);
  return stbi_write_hdr(filename, width, height, 3, data) != 0;
}
bool write_ppm(const char *filename, int width, int height, const float *data) {
  lerr("PPM file format has not yet been implemented, saving as BMP instead");
  return write_bmp(filename, width, height, data);
}
bool write_hex(const char *filename, int width, int height, const float *data) {
  ldebug("Writting HEX \"%s\"", filename);
  uint8_t *bits = (uint8_t *)malloc(width * height * 3);
  float_to_8bit(data, width * height * 3, bits);
  FILE *file = fopen(filename, "w");
  for (size_t i = 0; i < width * height; ++i) {
    if (i % width == 0)
      fprintf(file, "\n");
    fprintf(file, "%02x%02x%02x ", bits[3*i], bits[3*i+1], bits[3*i+2]);
  }
  fclose(file);
  free(bits);
  return true;
}

bool write_image(const char *filename, int width, int height,
                 const float *data) {
  char ext[8];
  stb_splitpath(ext, (char *)filename, STB_EXT);

  if (strncmp(ext, ".bmp", 4) == 0) {
    return write_bmp(filename, width, height, data);
  } else if (strncmp(ext, ".png", 4) == 0) {
    return write_png(filename, width, height, data);
  } else if (strncmp(ext, ".tga", 4) == 0) {
    return write_tga(filename, width, height, data);
  } else if (strncmp(ext, ".jpg", 4) == 0) {
    return write_jpg(filename, width, height, data);
  } else if (strncmp(ext, ".hdr", 4) == 0) {
    return write_hdr(filename, width, height, data);
  } else if (strncmp(ext, ".ppm", 4) == 0) {
    return write_ppm(filename, width, height, data);
  } else if (strncmp(ext, ".hex", 4) == 0) {
    return write_hex(filename, width, height, data);
  } else {
    lwarn("Invalid filename extension '%s'", ext);
  }
  return false;
}
