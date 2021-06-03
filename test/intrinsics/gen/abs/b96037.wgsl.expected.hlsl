void abs_b96037() {
  float res = abs(1.0f);
}

void vertex_main() {
  abs_b96037();
  return;
}

void fragment_main() {
  abs_b96037();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_b96037();
  return;
}

