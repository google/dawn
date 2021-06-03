void abs_1e9d53() {
  float2 res = abs(float2(0.0f, 0.0f));
}

void vertex_main() {
  abs_1e9d53();
  return;
}

void fragment_main() {
  abs_1e9d53();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_1e9d53();
  return;
}

