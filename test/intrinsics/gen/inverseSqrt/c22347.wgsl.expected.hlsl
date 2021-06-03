void inverseSqrt_c22347() {
  float4 res = rsqrt(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  inverseSqrt_c22347();
  return;
}

void fragment_main() {
  inverseSqrt_c22347();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  inverseSqrt_c22347();
  return;
}

