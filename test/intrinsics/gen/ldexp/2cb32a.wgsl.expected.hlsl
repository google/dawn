void ldexp_2cb32a() {
  float3 res = ldexp(float3(0.0f, 0.0f, 0.0f), uint3(0u, 0u, 0u));
}

void vertex_main() {
  ldexp_2cb32a();
  return;
}

void fragment_main() {
  ldexp_2cb32a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_2cb32a();
  return;
}

