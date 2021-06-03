void fract_943cb1() {
  float2 res = frac(float2(0.0f, 0.0f));
}

void vertex_main() {
  fract_943cb1();
  return;
}

void fragment_main() {
  fract_943cb1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fract_943cb1();
  return;
}

