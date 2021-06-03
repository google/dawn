void log_3da25a() {
  float4 res = log(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  log_3da25a();
  return;
}

void fragment_main() {
  log_3da25a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log_3da25a();
  return;
}

