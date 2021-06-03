void atan2_96057c() {
  float res = atan2(1.0f, 1.0f);
}

void vertex_main() {
  atan2_96057c();
  return;
}

void fragment_main() {
  atan2_96057c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_96057c();
  return;
}

