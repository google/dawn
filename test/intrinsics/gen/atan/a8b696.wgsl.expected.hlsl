void atan_a8b696() {
  float4 res = atan(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  atan_a8b696();
  return;
}

void fragment_main() {
  atan_a8b696();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_a8b696();
  return;
}

