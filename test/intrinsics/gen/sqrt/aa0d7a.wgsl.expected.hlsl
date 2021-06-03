void sqrt_aa0d7a() {
  float4 res = sqrt(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sqrt_aa0d7a();
  return;
}

void fragment_main() {
  sqrt_aa0d7a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_aa0d7a();
  return;
}

