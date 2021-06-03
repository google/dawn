void exp_1951e7() {
  float2 res = exp(float2(0.0f, 0.0f));
}

void vertex_main() {
  exp_1951e7();
  return;
}

void fragment_main() {
  exp_1951e7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_1951e7();
  return;
}

