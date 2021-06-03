void step_0b073b() {
  float res = step(1.0f, 1.0f);
}

void vertex_main() {
  step_0b073b();
  return;
}

void fragment_main() {
  step_0b073b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_0b073b();
  return;
}

