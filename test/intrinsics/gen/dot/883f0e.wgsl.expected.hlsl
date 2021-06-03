void dot_883f0e() {
  float res = dot(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  dot_883f0e();
  return;
}

void fragment_main() {
  dot_883f0e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_883f0e();
  return;
}

