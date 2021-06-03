void ceil_34064b() {
  float3 res = ceil(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  ceil_34064b();
  return;
}

void fragment_main() {
  ceil_34064b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_34064b();
  return;
}

