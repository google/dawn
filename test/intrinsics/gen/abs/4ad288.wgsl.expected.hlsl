void abs_4ad288() {
  int res = abs(1);
}

void vertex_main() {
  abs_4ad288();
  return;
}

void fragment_main() {
  abs_4ad288();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_4ad288();
  return;
}

