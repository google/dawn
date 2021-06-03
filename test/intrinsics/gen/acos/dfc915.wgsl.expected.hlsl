void acos_dfc915() {
  float2 res = acos(float2(0.0f, 0.0f));
}

void vertex_main() {
  acos_dfc915();
  return;
}

void fragment_main() {
  acos_dfc915();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_dfc915();
  return;
}

