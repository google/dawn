void clamp_b07c65() {
  int res = clamp(1, 1, 1);
}

void vertex_main() {
  clamp_b07c65();
  return;
}

void fragment_main() {
  clamp_b07c65();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_b07c65();
  return;
}

