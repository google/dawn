void isNan_e4978e() {
  bool res = isnan(1.0f);
}

void vertex_main() {
  isNan_e4978e();
  return;
}

void fragment_main() {
  isNan_e4978e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_e4978e();
  return;
}

