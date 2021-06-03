void tanh_9f9fb9() {
  float3 res = tanh(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  tanh_9f9fb9();
  return;
}

void fragment_main() {
  tanh_9f9fb9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_9f9fb9();
  return;
}

