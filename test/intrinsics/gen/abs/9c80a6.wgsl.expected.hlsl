void abs_9c80a6() {
  int4 res = abs(int4(0, 0, 0, 0));
}

void vertex_main() {
  abs_9c80a6();
  return;
}

void fragment_main() {
  abs_9c80a6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_9c80a6();
  return;
}

