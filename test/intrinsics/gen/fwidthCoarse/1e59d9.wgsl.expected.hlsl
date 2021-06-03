void fwidthCoarse_1e59d9() {
  float3 res = fwidth(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidthCoarse_1e59d9();
  return;
}

void fragment_main() {
  fwidthCoarse_1e59d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_1e59d9();
  return;
}

