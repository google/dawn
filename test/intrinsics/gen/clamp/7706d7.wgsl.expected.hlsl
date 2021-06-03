void clamp_7706d7() {
  uint2 res = clamp(uint2(0u, 0u), uint2(0u, 0u), uint2(0u, 0u));
}

void vertex_main() {
  clamp_7706d7();
  return;
}

void fragment_main() {
  clamp_7706d7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_7706d7();
  return;
}

