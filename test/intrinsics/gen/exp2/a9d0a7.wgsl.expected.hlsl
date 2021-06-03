void exp2_a9d0a7() {
  float4 res = exp2(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  exp2_a9d0a7();
  return;
}

void fragment_main() {
  exp2_a9d0a7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp2_a9d0a7();
  return;
}

