void asin_c0c272() {
  float res = asin(1.0f);
}

void vertex_main() {
  asin_c0c272();
  return;
}

void fragment_main() {
  asin_c0c272();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  asin_c0c272();
  return;
}

