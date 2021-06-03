void smoothStep_658be3() {
  float3 res = smoothstep(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  smoothStep_658be3();
  return;
}

void fragment_main() {
  smoothStep_658be3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_658be3();
  return;
}

