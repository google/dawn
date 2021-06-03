void max_453e04() {
  uint4 res = max(uint4(0u, 0u, 0u, 0u), uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  max_453e04();
  return;
}

void fragment_main() {
  max_453e04();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_453e04();
  return;
}

