void clamp_a2de25() {
  uint res = clamp(1u, 1u, 1u);
}

void vertex_main() {
  clamp_a2de25();
  return;
}

void fragment_main() {
  clamp_a2de25();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_a2de25();
  return;
}

