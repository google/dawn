void all_f46790() {
  bool res = all(vector<bool, 2>(false, false));
}

void vertex_main() {
  all_f46790();
  return;
}

void fragment_main() {
  all_f46790();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_f46790();
  return;
}

