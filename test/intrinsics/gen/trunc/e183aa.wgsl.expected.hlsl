void trunc_e183aa() {
  float4 res = trunc(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  trunc_e183aa();
  return;
}

void fragment_main() {
  trunc_e183aa();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_e183aa();
  return;
}

