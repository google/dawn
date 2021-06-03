void reflect_b61e10() {
  float2 res = reflect(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  reflect_b61e10();
  return;
}

void fragment_main() {
  reflect_b61e10();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_b61e10();
  return;
}

