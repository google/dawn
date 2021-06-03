void mix_0c8c33() {
  float3 res = lerp(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  mix_0c8c33();
  return;
}

void fragment_main() {
  mix_0c8c33();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_0c8c33();
  return;
}

