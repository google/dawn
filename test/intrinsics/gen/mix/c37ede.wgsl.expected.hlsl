void mix_c37ede() {
  float4 res = lerp(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  mix_c37ede();
  return;
}

void fragment_main() {
  mix_c37ede();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_c37ede();
  return;
}

