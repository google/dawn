void sqrt_f8c59a() {
  float3 res = sqrt(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sqrt_f8c59a();
  return;
}

void fragment_main() {
  sqrt_f8c59a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sqrt_f8c59a();
  return;
}

