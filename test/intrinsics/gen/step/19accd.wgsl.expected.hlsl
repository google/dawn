void step_19accd() {
  float2 res = step(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  step_19accd();
  return;
}

void fragment_main() {
  step_19accd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_19accd();
  return;
}

