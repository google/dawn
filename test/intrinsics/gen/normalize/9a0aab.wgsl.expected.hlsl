void normalize_9a0aab() {
  float4 res = normalize(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  normalize_9a0aab();
  return;
}

void fragment_main() {
  normalize_9a0aab();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  normalize_9a0aab();
  return;
}

