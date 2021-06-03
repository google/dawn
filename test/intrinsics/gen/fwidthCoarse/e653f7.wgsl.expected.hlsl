void fwidthCoarse_e653f7() {
  float2 res = fwidth(float2(0.0f, 0.0f));
}

void vertex_main() {
  fwidthCoarse_e653f7();
  return;
}

void fragment_main() {
  fwidthCoarse_e653f7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_e653f7();
  return;
}

