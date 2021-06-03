void fract_fa5c71() {
  float res = frac(1.0f);
}

void vertex_main() {
  fract_fa5c71();
  return;
}

void fragment_main() {
  fract_fa5c71();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_fa5c71();
  return;
}

