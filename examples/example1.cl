__kernal void tpm(write_only global float *image, read_only uint2 size) {
  for (int i = 0; i < size.x * size.y; ++i) {
    image[3 * i] = 1.0f;
    image[3 * i + 1] = 0.0f;
    image[3 * i + 2] = 1.0f;
  }
}

// vim:ft=c
