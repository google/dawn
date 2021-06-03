void distance_0657d4() {
  float res = distance(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  distance_0657d4();
  return;
}

void fragment_main() {
  distance_0657d4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_0657d4();
  return;
}

