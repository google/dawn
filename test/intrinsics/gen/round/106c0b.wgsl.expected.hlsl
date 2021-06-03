void round_106c0b() {
  float4 res = round(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  round_106c0b();
  return;
}

void fragment_main() {
  round_106c0b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_106c0b();
  return;
}

