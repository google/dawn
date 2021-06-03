void min_3941e1() {
  int4 res = min(int4(0, 0, 0, 0), int4(0, 0, 0, 0));
}

void vertex_main() {
  min_3941e1();
  return;
}

void fragment_main() {
  min_3941e1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_3941e1();
  return;
}

