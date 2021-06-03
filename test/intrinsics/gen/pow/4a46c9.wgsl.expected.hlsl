void pow_4a46c9() {
  float3 res = pow(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  pow_4a46c9();
  return;
}

void fragment_main() {
  pow_4a46c9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pow_4a46c9();
  return;
}

