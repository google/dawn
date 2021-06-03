void fract_a49758() {
  float3 res = frac(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fract_a49758();
  return;
}

void fragment_main() {
  fract_a49758();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_a49758();
  return;
}

