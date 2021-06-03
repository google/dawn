void sin_fc8bc4() {
  float2 res = sin(float2(0.0f, 0.0f));
}

void vertex_main() {
  sin_fc8bc4();
  return;
}

void fragment_main() {
  sin_fc8bc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_fc8bc4();
  return;
}

