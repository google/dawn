void clamp_2bd567() {
  float res = clamp(1.0f, 1.0f, 1.0f);
}

void vertex_main() {
  clamp_2bd567();
  return;
}

void fragment_main() {
  clamp_2bd567();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_2bd567();
  return;
}

