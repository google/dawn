void cos_16dc15() {
  float3 res = cos(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  cos_16dc15();
  return;
}

void fragment_main() {
  cos_16dc15();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cos_16dc15();
  return;
}

