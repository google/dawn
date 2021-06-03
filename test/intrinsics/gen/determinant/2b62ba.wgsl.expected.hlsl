void determinant_2b62ba() {
  float res = determinant(float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  determinant_2b62ba();
  return;
}

void fragment_main() {
  determinant_2b62ba();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  determinant_2b62ba();
  return;
}

