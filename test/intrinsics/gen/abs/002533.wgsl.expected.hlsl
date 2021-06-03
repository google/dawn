void abs_002533() {
  float4 res = abs(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  abs_002533();
  return;
}

void fragment_main() {
  abs_002533();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_002533();
  return;
}

