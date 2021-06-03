void dot_0c577b() {
  float res = dot(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  dot_0c577b();
  return;
}

void fragment_main() {
  dot_0c577b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_0c577b();
  return;
}

