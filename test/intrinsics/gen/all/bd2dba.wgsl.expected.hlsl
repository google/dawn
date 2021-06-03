void all_bd2dba() {
  bool res = all(vector<bool, 3>(false, false, false));
}

void vertex_main() {
  all_bd2dba();
  return;
}

void fragment_main() {
  all_bd2dba();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  all_bd2dba();
  return;
}

