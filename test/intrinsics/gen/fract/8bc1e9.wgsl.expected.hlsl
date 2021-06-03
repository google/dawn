void fract_8bc1e9() {
  float4 res = frac(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fract_8bc1e9();
  return;
}

void fragment_main() {
  fract_8bc1e9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_8bc1e9();
  return;
}

