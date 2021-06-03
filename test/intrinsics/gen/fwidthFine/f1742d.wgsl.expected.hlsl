void fwidthFine_f1742d() {
  float res = fwidth(1.0f);
}

void vertex_main() {
  fwidthFine_f1742d();
  return;
}

void fragment_main() {
  fwidthFine_f1742d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_f1742d();
  return;
}

