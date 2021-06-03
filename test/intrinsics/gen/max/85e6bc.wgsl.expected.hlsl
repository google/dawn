void max_85e6bc() {
  int4 res = max(int4(0, 0, 0, 0), int4(0, 0, 0, 0));
}

void vertex_main() {
  max_85e6bc();
  return;
}

void fragment_main() {
  max_85e6bc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_85e6bc();
  return;
}

