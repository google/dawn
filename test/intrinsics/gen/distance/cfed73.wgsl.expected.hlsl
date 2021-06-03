void distance_cfed73() {
  float res = distance(1.0f, 1.0f);
}

void vertex_main() {
  distance_cfed73();
  return;
}

void fragment_main() {
  distance_cfed73();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  distance_cfed73();
  return;
}

