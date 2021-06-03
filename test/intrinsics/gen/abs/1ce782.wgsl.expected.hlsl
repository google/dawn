void abs_1ce782() {
  uint4 res = abs(uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  abs_1ce782();
  return;
}

void fragment_main() {
  abs_1ce782();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_1ce782();
  return;
}

