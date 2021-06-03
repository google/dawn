void mix_4f0b5e() {
  float res = lerp(1.0f, 1.0f, 1.0f);
}

void vertex_main() {
  mix_4f0b5e();
  return;
}

void fragment_main() {
  mix_4f0b5e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  mix_4f0b5e();
  return;
}

