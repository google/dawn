void max_44a39d() {
  float res = max(1.0f, 1.0f);
}

void vertex_main() {
  max_44a39d();
  return;
}

void fragment_main() {
  max_44a39d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_44a39d();
  return;
}

