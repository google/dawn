void trunc_562d05() {
  float3 res = trunc(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  trunc_562d05();
  return;
}

void fragment_main() {
  trunc_562d05();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_562d05();
  return;
}

