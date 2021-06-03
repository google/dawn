void max_462050() {
  float2 res = max(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  max_462050();
  return;
}

void fragment_main() {
  max_462050();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_462050();
  return;
}

