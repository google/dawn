void smoothStep_c11eef() {
  float2 res = smoothstep(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  smoothStep_c11eef();
  return;
}

void fragment_main() {
  smoothStep_c11eef();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  smoothStep_c11eef();
  return;
}

