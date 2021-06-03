void fwidthFine_ff6aa0() {
  float2 res = fwidth(float2(0.0f, 0.0f));
}

void vertex_main() {
  fwidthFine_ff6aa0();
  return;
}

void fragment_main() {
  fwidthFine_ff6aa0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_ff6aa0();
  return;
}

