void asin_064953() {
  float4 res = asin(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  asin_064953();
  return;
}

void fragment_main() {
  asin_064953();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_064953();
  return;
}

