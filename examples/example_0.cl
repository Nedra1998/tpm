float sdSphere(float3 p, float r) { return length(p) - r; }


void sdf(float3 p, float* t, int* mat) {
    float tt = sdSphere(p, 1.0f);
    if(tt < *t) {
        *t = tt;
        *mat = 1;
    }
}

// vim:ft=opencl
