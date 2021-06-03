void clamp_0acf8f() {
  float2 res = clamp(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  clamp_0acf8f();
  return;
}

void fragment_main() {
  clamp_0acf8f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_0acf8f();
  return;
}

