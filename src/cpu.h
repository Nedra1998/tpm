#ifndef CPU_H_J5UM6RVE
#define CPU_H_J5UM6RVE

void path_trace(float *o, float *d, float *c);
void vec_normalize(const float in[3], float out[3]);
void vec_cross(const float a[3], const float b[3], float c[3]);
float vec_dot(const float a[3], const float b[3]);
void look_at(const float eye[3], const float at[3], const float up[3],
             float mat[16]);
void mat_inverse(const float in[16], float out[16]);
void mat_mult_point(const float mat[16], const float v[3], float out[3]);
void mat_mult_dir(const float mat[16], const float v[3], float out[3]);

#endif /* end of include guard: CPU_H_J5UM6RVE */
