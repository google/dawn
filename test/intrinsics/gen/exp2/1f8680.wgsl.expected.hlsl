void exp2_1f8680() {
  float3 res = exp2(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  exp2_1f8680();
  return;
}

void fragment_main() {
  exp2_1f8680();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp2_1f8680();
  return;
}

