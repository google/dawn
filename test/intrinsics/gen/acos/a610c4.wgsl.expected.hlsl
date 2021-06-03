void acos_a610c4() {
  float3 res = acos(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  acos_a610c4();
  return;
}

void fragment_main() {
  acos_a610c4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_a610c4();
  return;
}

