void abs_467cd1() {
  uint res = abs(1u);
}

void vertex_main() {
  abs_467cd1();
  return;
}

void fragment_main() {
  abs_467cd1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_467cd1();
  return;
}

