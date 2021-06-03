void abs_7326de() {
  uint3 res = abs(uint3(0u, 0u, 0u));
}

void vertex_main() {
  abs_7326de();
  return;
}

void fragment_main() {
  abs_7326de();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_7326de();
  return;
}

