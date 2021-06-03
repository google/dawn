void any_083428() {
  bool res = any(vector<bool, 4>(false, false, false, false));
}

void vertex_main() {
  any_083428();
  return;
}

void fragment_main() {
  any_083428();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  any_083428();
  return;
}

