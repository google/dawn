void clamp_5f0819() {
  int3 res = clamp(int3(0, 0, 0), int3(0, 0, 0), int3(0, 0, 0));
}

void vertex_main() {
  clamp_5f0819();
  return;
}

void fragment_main() {
  clamp_5f0819();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_5f0819();
  return;
}

