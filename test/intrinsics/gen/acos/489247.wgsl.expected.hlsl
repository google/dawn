void acos_489247() {
  float res = acos(1.0f);
}

void vertex_main() {
  acos_489247();
  return;
}

void fragment_main() {
  acos_489247();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_489247();
  return;
}

