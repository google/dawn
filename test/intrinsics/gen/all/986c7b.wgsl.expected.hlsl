void all_986c7b() {
  bool res = all(vector<bool, 4>(false, false, false, false));
}

void vertex_main() {
  all_986c7b();
  return;
}

void fragment_main() {
  all_986c7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_986c7b();
  return;
}

