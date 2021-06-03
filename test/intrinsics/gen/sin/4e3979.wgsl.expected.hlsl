void sin_4e3979() {
  float4 res = sin(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sin_4e3979();
  return;
}

void fragment_main() {
  sin_4e3979();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_4e3979();
  return;
}

