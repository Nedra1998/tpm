typedef struct Res {
  float t;
  ulong m;
} Res;

Res opU(Res d1, Res d2) { return (d1.t < d2.t) ? d1 : d2; }

float sdSphere(float3 p, float r) { return length(p) - r; }
float sdBox(float3 p, float3 b) {
  float3 q = fabs(p) - b;
  return length(max(q, 0.0f)) + min(max(q.x, max(q.y, q.z)), 0.0f);
}

// vim:ft=opencl
