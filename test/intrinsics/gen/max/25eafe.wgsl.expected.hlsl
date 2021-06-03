void max_25eafe() {
  int3 res = max(int3(0, 0, 0), int3(0, 0, 0));
}

void vertex_main() {
  max_25eafe();
  return;
}

void fragment_main() {
  max_25eafe();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_25eafe();
  return;
}

