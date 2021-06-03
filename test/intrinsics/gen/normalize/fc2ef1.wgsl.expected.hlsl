void normalize_fc2ef1() {
  float2 res = normalize(float2(0.0f, 0.0f));
}

void vertex_main() {
  normalize_fc2ef1();
  return;
}

void fragment_main() {
  normalize_fc2ef1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_fc2ef1();
  return;
}

