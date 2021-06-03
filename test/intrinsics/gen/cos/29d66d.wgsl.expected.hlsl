void cos_29d66d() {
  float4 res = cos(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  cos_29d66d();
  return;
}

void fragment_main() {
  cos_29d66d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cos_29d66d();
  return;
}

