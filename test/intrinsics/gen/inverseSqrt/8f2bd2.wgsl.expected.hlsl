void inverseSqrt_8f2bd2() {
  float2 res = rsqrt(float2(0.0f, 0.0f));
}

void vertex_main() {
  inverseSqrt_8f2bd2();
  return;
}

void fragment_main() {
  inverseSqrt_8f2bd2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  inverseSqrt_8f2bd2();
  return;
}

