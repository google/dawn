void abs_5ad50a() {
  int3 res = abs(int3(0, 0, 0));
}

void vertex_main() {
  abs_5ad50a();
  return;
}

void fragment_main() {
  abs_5ad50a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_5ad50a();
  return;
}

