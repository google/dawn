void clamp_1a32e3() {
  int4 res = clamp(int4(0, 0, 0, 0), int4(0, 0, 0, 0), int4(0, 0, 0, 0));
}

void vertex_main() {
  clamp_1a32e3();
  return;
}

void fragment_main() {
  clamp_1a32e3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_1a32e3();
  return;
}

