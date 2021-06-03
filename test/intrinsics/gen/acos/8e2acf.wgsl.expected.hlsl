void acos_8e2acf() {
  float4 res = acos(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  acos_8e2acf();
  return;
}

void fragment_main() {
  acos_8e2acf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_8e2acf();
  return;
}

