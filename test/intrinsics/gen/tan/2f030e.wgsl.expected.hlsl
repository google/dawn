void tan_2f030e() {
  float res = tan(1.0f);
}

void vertex_main() {
  tan_2f030e();
  return;
}

void fragment_main() {
  tan_2f030e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_2f030e();
  return;
}

