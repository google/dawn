void fwidthFine_68f4ef() {
  float4 res = fwidth(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidthFine_68f4ef();
  return;
}

void fragment_main() {
  fwidthFine_68f4ef();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidthFine_68f4ef();
  return;
}

