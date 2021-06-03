void dot_ba4246() {
  float res = dot(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  dot_ba4246();
  return;
}

void fragment_main() {
  dot_ba4246();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_ba4246();
  return;
}

