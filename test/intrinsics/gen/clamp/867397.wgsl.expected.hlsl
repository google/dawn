void clamp_867397() {
  float3 res = clamp(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  clamp_867397();
  return;
}

void fragment_main() {
  clamp_867397();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_867397();
  return;
}

