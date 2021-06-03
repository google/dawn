void round_1c7897() {
  float3 res = round(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  round_1c7897();
  return;
}

void fragment_main() {
  round_1c7897();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_1c7897();
  return;
}

