void determinant_a0a87c() {
  float res = determinant(float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  determinant_a0a87c();
  return;
}

void fragment_main() {
  determinant_a0a87c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  determinant_a0a87c();
  return;
}

