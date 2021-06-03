void tanh_5724b3() {
  float2 res = tanh(float2(0.0f, 0.0f));
}

void vertex_main() {
  tanh_5724b3();
  return;
}

void fragment_main() {
  tanh_5724b3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_5724b3();
  return;
}

