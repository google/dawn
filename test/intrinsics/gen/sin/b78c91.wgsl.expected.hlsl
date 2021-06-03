void sin_b78c91() {
  float res = sin(1.0f);
}

void vertex_main() {
  sin_b78c91();
  return;
}

void fragment_main() {
  sin_b78c91();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sin_b78c91();
  return;
}

