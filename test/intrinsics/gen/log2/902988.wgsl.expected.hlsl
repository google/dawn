void log2_902988() {
  float4 res = log2(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  log2_902988();
  return;
}

void fragment_main() {
  log2_902988();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_902988();
  return;
}

