void atan_02979a() {
  float res = atan(1.0f);
}

void vertex_main() {
  atan_02979a();
  return;
}

void fragment_main() {
  atan_02979a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan_02979a();
  return;
}

