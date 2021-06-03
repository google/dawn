void sin_01f241() {
  float3 res = sin(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sin_01f241();
  return;
}

void fragment_main() {
  sin_01f241();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_01f241();
  return;
}

