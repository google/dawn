void fwidthFine_523fdc() {
  float3 res = fwidth(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidthFine_523fdc();
  return;
}

void fragment_main() {
  fwidthFine_523fdc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_523fdc();
  return;
}

