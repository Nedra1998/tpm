#ifndef IMAGE_H_WJCZR80A
#define IMAGE_H_WJCZR80A

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void float_to_8bit(const float* in, int size, uint8_t* out);

bool write_bmp(const char* filename, int width, int height, const float* data);
bool write_png(const char* filename, int width, int height, const float* data);
bool write_tga(const char* filename, int width, int height, const float* data);
bool write_jpg(const char* filename, int width, int height, const float* data);
bool write_hdr(const char* filename, int width, int height, const float* data);
bool write_ppm(const char* filename, int width, int height, const float* data);
bool write_hex(const char* filename, int width, int height, const float* data);

bool write_image(const char* filename, int width, int height, const float* data);

#endif /* end of include guard: IMAGE_H_WJCZR80A */
