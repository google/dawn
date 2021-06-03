void distance_aa4055() {
  float res = distance(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  distance_aa4055();
  return;
}

void fragment_main() {
  distance_aa4055();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_aa4055();
  return;
}

