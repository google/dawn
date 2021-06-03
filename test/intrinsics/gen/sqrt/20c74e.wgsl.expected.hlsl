void sqrt_20c74e() {
  float res = sqrt(1.0f);
}

void vertex_main() {
  sqrt_20c74e();
  return;
}

void fragment_main() {
  sqrt_20c74e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_20c74e();
  return;
}

