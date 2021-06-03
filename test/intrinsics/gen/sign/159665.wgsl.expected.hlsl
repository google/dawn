void sign_159665() {
  float3 res = sign(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sign_159665();
  return;
}

void fragment_main() {
  sign_159665();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_159665();
  return;
}

