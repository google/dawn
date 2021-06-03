void exp2_d6777c() {
  float2 res = exp2(float2(0.0f, 0.0f));
}

void vertex_main() {
  exp2_d6777c();
  return;
}

void fragment_main() {
  exp2_d6777c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp2_d6777c();
  return;
}

