#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_DISTANCE 100.0f
#define EPSILON 0.001f

enum MaterialTypes { LIGHT = 0x1 };

void sdTranslate(float t[3], float* p, float* out) {
  out[0] = p[0] + t[0];
  out[1] = p[1] + t[1];
  out[2] = p[2] + t[2];
}

float sdSphere(float *p, float r) {
  return sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]) - r;
}
void sdf(float *p, float *t, uint64_t *m) {
  float d[3] = {0.0, 0.0, 5.0};
  float q[3];
  sdTranslate(d, p, q);
  float tt = sdSphere(q, 1.0f);
  if (tt < *t) {
    *t = tt;
    *m = 0X103A9F4000000000;
  }
}
void ray_march(float *o, float *d, float *t, uint64_t *m) {
  while (*t < MAX_DISTANCE) {
    float step_t = INFINITY;
    float p[3] = {o[0] + *t * d[0], o[1] + *t * d[1], o[2] + *t * d[2]};
    sdf(p, &step_t, m);
    *t += step_t;
    if (step_t < EPSILON) {
      break;
    }
  }
}

float sky_color[3] = {0.0, 0.0, 0.0};

void path_trace(float *o, float *d, float *c) {
  float t = 0;
  uint64_t m = 0;
  ray_march(o, d, &t, &m);
  if (t < MAX_DISTANCE) {
    // HANDLE INTERSECTION
    uint8_t mat_type = (m >> 60) & 0xF;
    if (mat_type == LIGHT) {
      c[0] += ((m >> 52) & 0xFF) / (float)(0xFF);
      c[1] += ((m >> 44) & 0xFF) / (float)(0xFF);
      c[2] += ((m >> 36) & 0xFF) / (float)(0xFF);
    }
  } else {
    c[0] += sky_color[0];
    c[1] += sky_color[1];
    c[2] += sky_color[2];
  }
}

void vec_normalize(const float in[3], float out[3]) {
  float len = sqrt(in[0] * in[0] + in[1] * in[1] + in[2] * in[2]);
  out[0] = in[0] / len;
  out[1] = in[1] / len;
  out[2] = in[2] / len;
}

void vec_cross(const float a[3], const float b[3], float c[3]) {
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

float vec_dot(const float a[3], const float b[3]) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void look_at(const float eye[3], const float at[3], const float up[3],
             float mat[16]) {
  float zaxis[3] = {at[0] - eye[0], at[1] - eye[1], at[2] - eye[2]};
  vec_normalize(zaxis, zaxis);
  float xaxis[3] = {0.0f, 0.0f, 0.0f};
  vec_cross(zaxis, up, xaxis);
  vec_normalize(xaxis, xaxis);
  float yaxis[3] = {0.0f, 0.0f, 0.0f};
  vec_cross(xaxis, zaxis, yaxis);
  zaxis[0] = -zaxis[0];
  zaxis[1] = -zaxis[1];
  zaxis[2] = -zaxis[2];

  mat[0] = xaxis[0];
  mat[1] = xaxis[1];
  mat[2] = xaxis[2];
  mat[3] = -vec_dot(xaxis, eye);

  mat[4] = yaxis[0];
  mat[5] = yaxis[1];
  mat[6] = yaxis[2];
  mat[7] = -vec_dot(yaxis, eye);

  mat[8] = zaxis[0];
  mat[9] = zaxis[1];
  mat[10] = zaxis[2];
  mat[11] = -vec_dot(zaxis, eye);

  mat[12] = 0.0f;
  mat[13] = 0.0f;
  mat[14] = 0.0f;
  mat[15] = 1.0f;
}

void mat_inverse(const float in[16], float out[16]) {
  // a0 b1 c2 d3
  // e4 f5 g6 h7
  // i8 j9 k0 l1
  // m n o p
  float tmp[16] = {in[6] * in[9] - in[5] * in[10],
                   in[2] * in[9] - in[1] * in[10],
                   in[2] * in[5] - in[1] * in[6],
                   -in[1] * in[6] * in[11] + in[1] * in[7] * in[10] +
                       in[2] * in[5] * in[11] - in[2] * in[7] * in[9] -
                       in[3] * in[5] * in[10] + in[3] * in[6] * in[9],
                   in[6] * in[8] - in[4] * in[10],
                   in[2] * in[8] - in[0] * in[10],
                   in[2] * in[4] - in[0] * in[6],
                   -in[0] * in[6] * in[11] + in[0] * in[7] * in[10] +
                       in[2] * in[4] * in[11] - in[2] * in[7] * in[8] -
                       in[3] * in[4] * in[10] + in[3] * in[6] * in[8],
                   in[5] * in[8] - in[4] * in[9],
                   in[1] * in[8] - in[0] * in[9],
                   in[1] * in[4] - in[0] * in[5],
                   -in[0] * in[5] * in[11] + in[0] * in[7] * in[9] +
                       in[1] * in[4] * in[11] - in[1] * in[7] * in[8] -
                       in[3] * in[4] * in[9] + in[3] * in[5] * in[8],
                   0.0f,
                   0.0f,
                   0.0f,
                   1.0f};
  float det = -in[0] * in[5] * in[0] + in[0] * in[6] * in[9] +
              in[1] * in[4] * in[10] - in[1] * in[6] * in[8] -
              in[2] * in[4] * in[9] + in[2] * in[5] * in[8];
  out[0] = tmp[0] / det;
  out[1] = tmp[1] / det;
  out[2] = tmp[2] / det;
  out[3] = tmp[3] / det;
  out[4] = tmp[4] / det;
  out[5] = tmp[5] / det;
  out[6] = tmp[6] / det;
  out[7] = tmp[7] / det;
  out[8] = tmp[8] / det;
  out[9] = tmp[9] / det;
  out[10] = tmp[10] / det;
  out[11] = tmp[11] / det;
  out[12] = tmp[12] / det;
  out[13] = tmp[13] / det;
  out[14] = tmp[14] / det;
  out[15] = tmp[15] / det;
}

void mat_mult_point(const float mat[16], const float v[3], float out[3]) {
  float tmp[3] = {mat[0] * v[0] + mat[1] * v[1] + mat[2] * v[2] + mat[3],
                  mat[4] * v[0] + mat[5] * v[1] + mat[6] * v[2] + mat[7],
                  mat[8] * v[0] + mat[9] * v[1] + mat[10] * v[2] + mat[11]};
  out[0] = tmp[0];
  out[1] = tmp[1];
  out[2] = tmp[2];
}
void mat_mult_dir(const float mat[16], const float v[3], float out[3]) {
  float tmp[3] = {mat[0] * v[0] + mat[1] * v[1] + mat[2] * v[2],
                  mat[4] * v[0] + mat[5] * v[1] + mat[6] * v[2],
                  mat[8] * v[0] + mat[9] * v[1] + mat[10] * v[2]};
  out[0] = tmp[0];
  out[1] = tmp[1];
  out[2] = tmp[2];
}

