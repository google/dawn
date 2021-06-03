void ldexp_cc9cde() {
  float4 res = ldexp(float4(0.0f, 0.0f, 0.0f, 0.0f), int4(0, 0, 0, 0));
}

void vertex_main() {
  ldexp_cc9cde();
  return;
}

void fragment_main() {
  ldexp_cc9cde();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_cc9cde();
  return;
}

