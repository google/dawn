void reflect_05357e() {
  float4 res = reflect(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  reflect_05357e();
  return;
}

void fragment_main() {
  reflect_05357e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_05357e();
  return;
}

