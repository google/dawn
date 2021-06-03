void min_c76fa6() {
  float4 res = min(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  min_c76fa6();
  return;
}

void fragment_main() {
  min_c76fa6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_c76fa6();
  return;
}

