void min_0dc614() {
  uint4 res = min(uint4(0u, 0u, 0u, 0u), uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  min_0dc614();
  return;
}

void fragment_main() {
  min_0dc614();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_0dc614();
  return;
}

