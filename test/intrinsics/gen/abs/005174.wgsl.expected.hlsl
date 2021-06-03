void abs_005174() {
  float3 res = abs(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  abs_005174();
  return;
}

void fragment_main() {
  abs_005174();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_005174();
  return;
}

