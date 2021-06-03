void ldexp_abd718() {
  float2 res = ldexp(float2(0.0f, 0.0f), int2(0, 0));
}

void vertex_main() {
  ldexp_abd718();
  return;
}

void fragment_main() {
  ldexp_abd718();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_abd718();
  return;
}

