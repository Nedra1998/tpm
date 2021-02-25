__constant float MAX_DISTANCE = 100.0f;
__constant float EPSILON = 0.01f;

float16 inverse(const float16 m) {
  float16 inv;
  float det;

  inv.s0 = m.s5 * m.sa * m.sf - m.s5 * m.sb * m.se - m.s9 * m.s6 * m.sf +
           m.s9 * m.s7 * m.se + m.sd * m.s6 * m.sb - m.sd * m.s7 * m.sa;

  inv.s4 = -m.s4 * m.sa * m.sf + m.s4 * m.sb * m.se + m.s8 * m.s6 * m.sf -
           m.s8 * m.s7 * m.se - m.sc * m.s6 * m.sb + m.sc * m.s7 * m.sa;

  inv.s8 = m.s4 * m.s9 * m.sf - m.s4 * m.sb * m.sd - m.s8 * m.s5 * m.sf +
           m.s8 * m.s7 * m.sd + m.sc * m.s5 * m.sb - m.sc * m.s7 * m.s9;

  inv.sc = -m.s4 * m.s9 * m.se + m.s4 * m.sa * m.sd + m.s8 * m.s5 * m.se -
           m.s8 * m.s6 * m.sd - m.sc * m.s5 * m.sa + m.sc * m.s6 * m.s9;

  inv.s1 = -m.s1 * m.sa * m.sf + m.s1 * m.sb * m.se + m.s9 * m.s2 * m.sf -
           m.s9 * m.s3 * m.se - m.sd * m.s2 * m.sb + m.sd * m.s3 * m.sa;

  inv.s5 = m.s0 * m.sa * m.sf - m.s0 * m.sb * m.se - m.s8 * m.s2 * m.sf +
           m.s8 * m.s3 * m.se + m.sc * m.s2 * m.sb - m.sc * m.s3 * m.sa;

  inv.s9 = -m.s0 * m.s9 * m.sf + m.s0 * m.sb * m.sd + m.s8 * m.s1 * m.sf -
           m.s8 * m.s3 * m.sd - m.sc * m.s1 * m.sb + m.sc * m.s3 * m.s9;

  inv.sd = m.s0 * m.s9 * m.se - m.s0 * m.sa * m.sd - m.s8 * m.s1 * m.se +
           m.s8 * m.s2 * m.sd + m.sc * m.s1 * m.sa - m.sc * m.s2 * m.s9;

  inv.s2 = m.s1 * m.s6 * m.sf - m.s1 * m.s7 * m.se - m.s5 * m.s2 * m.sf +
           m.s5 * m.s3 * m.se + m.sd * m.s2 * m.s7 - m.sd * m.s3 * m.s6;

  inv.s6 = -m.s0 * m.s6 * m.sf + m.s0 * m.s7 * m.se + m.s4 * m.s2 * m.sf -
           m.s4 * m.s3 * m.se - m.sc * m.s2 * m.s7 + m.sc * m.s3 * m.s6;

  inv.sa = m.s0 * m.s5 * m.sf - m.s0 * m.s7 * m.sd - m.s4 * m.s1 * m.sf +
           m.s4 * m.s3 * m.sd + m.sc * m.s1 * m.s7 - m.sc * m.s3 * m.s5;

  inv.se = -m.s0 * m.s5 * m.se + m.s0 * m.s6 * m.sd + m.s4 * m.s1 * m.se -
           m.s4 * m.s2 * m.sd - m.sc * m.s1 * m.s6 + m.sc * m.s2 * m.s5;

  inv.s3 = -m.s1 * m.s6 * m.sb + m.s1 * m.s7 * m.sa + m.s5 * m.s2 * m.sb -
           m.s5 * m.s3 * m.sa - m.s9 * m.s2 * m.s7 + m.s9 * m.s3 * m.s6;

  inv.s7 = m.s0 * m.s6 * m.sb - m.s0 * m.s7 * m.sa - m.s4 * m.s2 * m.sb +
           m.s4 * m.s3 * m.sa + m.s8 * m.s2 * m.s7 - m.s8 * m.s3 * m.s6;

  inv.sb = -m.s0 * m.s5 * m.sb + m.s0 * m.s7 * m.s9 + m.s4 * m.s1 * m.sb -
           m.s4 * m.s3 * m.s9 - m.s8 * m.s1 * m.s7 + m.s8 * m.s3 * m.s5;

  inv.sf = m.s0 * m.s5 * m.sa - m.s0 * m.s6 * m.s9 - m.s4 * m.s1 * m.sa +
           m.s4 * m.s2 * m.s9 + m.s8 * m.s1 * m.s6 - m.s8 * m.s2 * m.s5;

  det = 1.0 / (m.s0 * inv.s0 + m.s1 * inv.s4 + m.s2 * inv.s8 + m.s3 * inv.sc);
  return inv;
}

float4 mult(const float16 m, float4 v) {
  return (float4)(m.s0123 * v, m.s4567 * v, m.s890a * v, m.sbcde * v);
}

typedef struct Res {
  float t;
  ulong m;
} Res;

Res opU(Res d1, Res d2) { return (d1.t < d2.t) ? d1 : d2; }

float sdSphere(float3 p, float r) { return length(p) - r; }

Res sdf(float3 pos) {
  Res res = {INFINITY, 0x0};
  re = opU(res, (float2)(sdSphere(pos - (float3)(0.0, 0.0, 5.0)),
                         0X103A9F4000000000));
  return res;
}

Res rayMarch(float3 o, float3 d) {
    Res res = {0.0f, 0x0};
    while(res.t < MAX_DISTANCE) {
        float3 p = o + res.t * d;
        Res sdf_res = sdf(p);
        res.t += sdf_res.t;
        res.m = sdf_res.m;
        if(sdf_res.t < EPSILON) break;
    }
    return res;
}

float4 pathTrace(float3 o, float3 d) {
  Res res = rayMarch(o, d);
  if (t < MAX_DISTANCE) {
    uchar = mat_type = (m >> 60) & 0xF;
    switch (mat_type) {
    case 0x1:
      return (float4)(((m >> 52) & 0xFF) / (float)(0xFF)((m >> 44) & 0xFF) /
                      (float)(0xFF)((m >> 36) & 0xFF) / (float)(0xFF));
      break;
    default:
      return (float4)(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
  }
  return (float4)(1.0f, 0.0f, 0.0f, 1.0f);
}

__kernel void renderImage(__global __read_only image2d_t img,
                          __global __read_only float16 view) {
  int2 resolution = get_image_dim(img);
  float filmz = resolution.x / (2.0f * tan(1.5707f / 2.0f));
  size_t step_size = get_num_groups(0);
  float3 origin = mult(view, (float4)(0.0, 0.0, 0.0, 1.0));

  for (size_t i = get_global_id(0); i < resolution.x * resolution.y;
       i += step_size) {
    // int2 coord = (int2)(i % resolution.x, i / resolution.x);
    float4 color = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    // float3 direction = normalize(
    //     mult(view, (float4)(coord.x - resolution.x / 2.0f,
    //                         coord.y - resolution.y / 2.0f, filmz, 0.0f)));
    // color += pathTrace(origin, direction);
    write_imagef(img, coord, color);
  }
}

// vim:ft=opencl
