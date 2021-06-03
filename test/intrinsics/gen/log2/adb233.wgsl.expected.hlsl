void log2_adb233() {
  float3 res = log2(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  log2_adb233();
  return;
}

void fragment_main() {
  log2_adb233();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_adb233();
  return;
}

