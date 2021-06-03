void smoothStep_5f615b() {
  float4 res = smoothstep(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  smoothStep_5f615b();
  return;
}

void fragment_main() {
  smoothStep_5f615b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_5f615b();
  return;
}

