void fwidthCoarse_4e4fc4() {
  float4 res = fwidth(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidthCoarse_4e4fc4();
  return;
}

void fragment_main() {
  fwidthCoarse_4e4fc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_4e4fc4();
  return;
}

