void fwidth_d2ab9a() {
  float4 res = fwidth(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fwidth_d2ab9a();
  return;
}

void fragment_main() {
  fwidth_d2ab9a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_d2ab9a();
  return;
}

