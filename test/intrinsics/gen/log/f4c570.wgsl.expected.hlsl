void log_f4c570() {
  float3 res = log(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  log_f4c570();
  return;
}

void fragment_main() {
  log_f4c570();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_f4c570();
  return;
}

