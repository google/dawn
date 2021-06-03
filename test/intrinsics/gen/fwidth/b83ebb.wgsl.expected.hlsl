void fwidth_b83ebb() {
  float2 res = fwidth(float2(0.0f, 0.0f));
}

void vertex_main() {
  fwidth_b83ebb();
  return;
}

void fragment_main() {
  fwidth_b83ebb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_b83ebb();
  return;
}

