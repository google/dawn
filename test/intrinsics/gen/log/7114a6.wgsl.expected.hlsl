void log_7114a6() {
  float res = log(1.0f);
}

void vertex_main() {
  log_7114a6();
  return;
}

void fragment_main() {
  log_7114a6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_7114a6();
  return;
}

