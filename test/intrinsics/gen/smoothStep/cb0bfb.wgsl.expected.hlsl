void smoothStep_cb0bfb() {
  float res = smoothstep(1.0f, 1.0f, 1.0f);
}

void vertex_main() {
  smoothStep_cb0bfb();
  return;
}

void fragment_main() {
  smoothStep_cb0bfb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_cb0bfb();
  return;
}

