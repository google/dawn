void ldexp_4d6f6d() {
  float4 res = ldexp(float4(0.0f, 0.0f, 0.0f, 0.0f), uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  ldexp_4d6f6d();
  return;
}

void fragment_main() {
  ldexp_4d6f6d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_4d6f6d();
  return;
}

