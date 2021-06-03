void asin_8cd9c9() {
  float3 res = asin(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  asin_8cd9c9();
  return;
}

void fragment_main() {
  asin_8cd9c9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_8cd9c9();
  return;
}

