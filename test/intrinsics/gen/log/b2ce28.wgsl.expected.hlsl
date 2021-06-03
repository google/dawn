void log_b2ce28() {
  float2 res = log(float2(0.0f, 0.0f));
}

void vertex_main() {
  log_b2ce28();
  return;
}

void fragment_main() {
  log_b2ce28();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_b2ce28();
  return;
}

