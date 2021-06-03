void ldexp_a31cdc() {
  float3 res = ldexp(float3(0.0f, 0.0f, 0.0f), int3(0, 0, 0));
}

void vertex_main() {
  ldexp_a31cdc();
  return;
}

void fragment_main() {
  ldexp_a31cdc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_a31cdc();
  return;
}

