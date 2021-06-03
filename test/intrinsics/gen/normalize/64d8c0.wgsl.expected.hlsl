void normalize_64d8c0() {
  float3 res = normalize(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  normalize_64d8c0();
  return;
}

void fragment_main() {
  normalize_64d8c0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_64d8c0();
  return;
}

