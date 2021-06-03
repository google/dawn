void fwidth_5d1b39() {
  float3 res = fwidth(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidth_5d1b39();
  return;
}

void fragment_main() {
  fwidth_5d1b39();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_5d1b39();
  return;
}

