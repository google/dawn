void min_c70bb7() {
  uint3 res = min(uint3(0u, 0u, 0u), uint3(0u, 0u, 0u));
}

void vertex_main() {
  min_c70bb7();
  return;
}

void fragment_main() {
  min_c70bb7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_c70bb7();
  return;
}

