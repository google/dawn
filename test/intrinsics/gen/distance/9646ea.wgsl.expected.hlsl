void distance_9646ea() {
  float res = distance(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  distance_9646ea();
  return;
}

void fragment_main() {
  distance_9646ea();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_9646ea();
  return;
}

