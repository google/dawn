void min_93cfc4() {
  float3 res = min(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  min_93cfc4();
  return;
}

void fragment_main() {
  min_93cfc4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_93cfc4();
  return;
}

