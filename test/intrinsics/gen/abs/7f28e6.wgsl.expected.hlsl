void abs_7f28e6() {
  uint2 res = abs(uint2(0u, 0u));
}

void vertex_main() {
  abs_7f28e6();
  return;
}

void fragment_main() {
  abs_7f28e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_7f28e6();
  return;
}

