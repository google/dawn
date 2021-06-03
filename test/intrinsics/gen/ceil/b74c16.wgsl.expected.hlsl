void ceil_b74c16() {
  float4 res = ceil(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  ceil_b74c16();
  return;
}

void fragment_main() {
  ceil_b74c16();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_b74c16();
  return;
}

