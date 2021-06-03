void clamp_2bde41() {
  float4 res = clamp(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  clamp_2bde41();
  return;
}

void fragment_main() {
  clamp_2bde41();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_2bde41();
  return;
}

