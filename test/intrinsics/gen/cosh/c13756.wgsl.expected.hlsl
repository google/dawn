void cosh_c13756() {
  float2 res = cosh(float2(0.0f, 0.0f));
}

void vertex_main() {
  cosh_c13756();
  return;
}

void fragment_main() {
  cosh_c13756();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_c13756();
  return;
}

