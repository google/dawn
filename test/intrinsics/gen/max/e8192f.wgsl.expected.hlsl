void max_e8192f() {
  int2 res = max(int2(0, 0), int2(0, 0));
}

void vertex_main() {
  max_e8192f();
  return;
}

void fragment_main() {
  max_e8192f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_e8192f();
  return;
}

