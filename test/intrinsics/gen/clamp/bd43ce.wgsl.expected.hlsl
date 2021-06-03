void clamp_bd43ce() {
  uint4 res = clamp(uint4(0u, 0u, 0u, 0u), uint4(0u, 0u, 0u, 0u), uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  clamp_bd43ce();
  return;
}

void fragment_main() {
  clamp_bd43ce();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_bd43ce();
  return;
}

