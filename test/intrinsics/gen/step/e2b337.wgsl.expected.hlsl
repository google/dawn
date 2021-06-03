void step_e2b337() {
  float4 res = step(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  step_e2b337();
  return;
}

void fragment_main() {
  step_e2b337();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_e2b337();
  return;
}

