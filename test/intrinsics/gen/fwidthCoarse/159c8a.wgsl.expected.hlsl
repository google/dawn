void fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
}

void vertex_main() {
  fwidthCoarse_159c8a();
  return;
}

void fragment_main() {
  fwidthCoarse_159c8a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthCoarse_159c8a();
  return;
}

