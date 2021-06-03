void any_e755c1() {
  bool res = any(vector<bool, 3>(false, false, false));
}

void vertex_main() {
  any_e755c1();
  return;
}

void fragment_main() {
  any_e755c1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  any_e755c1();
  return;
}

