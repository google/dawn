void max_b1b73a() {
  uint3 res = max(uint3(0u, 0u, 0u), uint3(0u, 0u, 0u));
}

void vertex_main() {
  max_b1b73a();
  return;
}

void fragment_main() {
  max_b1b73a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_b1b73a();
  return;
}

