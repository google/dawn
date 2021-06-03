void mix_6f8adc() {
  float2 res = lerp(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  mix_6f8adc();
  return;
}

void fragment_main() {
  mix_6f8adc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_6f8adc();
  return;
}

