void exp2_dea523() {
  float res = exp2(1.0f);
}

void vertex_main() {
  exp2_dea523();
  return;
}

void fragment_main() {
  exp2_dea523();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp2_dea523();
  return;
}

