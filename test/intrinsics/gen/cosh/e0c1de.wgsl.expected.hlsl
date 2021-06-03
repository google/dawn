void cosh_e0c1de() {
  float4 res = cosh(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  cosh_e0c1de();
  return;
}

void fragment_main() {
  cosh_e0c1de();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_e0c1de();
  return;
}

