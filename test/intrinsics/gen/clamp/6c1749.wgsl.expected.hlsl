void clamp_6c1749() {
  int2 res = clamp(int2(0, 0), int2(0, 0), int2(0, 0));
}

void vertex_main() {
  clamp_6c1749();
  return;
}

void fragment_main() {
  clamp_6c1749();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_6c1749();
  return;
}

