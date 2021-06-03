void exp_771fd2() {
  float res = exp(1.0f);
}

void vertex_main() {
  exp_771fd2();
  return;
}

void fragment_main() {
  exp_771fd2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_771fd2();
  return;
}

