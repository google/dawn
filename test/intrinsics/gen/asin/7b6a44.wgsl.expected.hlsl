void asin_7b6a44() {
  float2 res = asin(float2(0.0f, 0.0f));
}

void vertex_main() {
  asin_7b6a44();
  return;
}

void fragment_main() {
  asin_7b6a44();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_7b6a44();
  return;
}

