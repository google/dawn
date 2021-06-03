void pow_04a908() {
  float4 res = pow(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  pow_04a908();
  return;
}

void fragment_main() {
  pow_04a908();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  pow_04a908();
  return;
}

