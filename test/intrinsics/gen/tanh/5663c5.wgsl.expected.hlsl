void tanh_5663c5() {
  float4 res = tanh(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  tanh_5663c5();
  return;
}

void fragment_main() {
  tanh_5663c5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_5663c5();
  return;
}

