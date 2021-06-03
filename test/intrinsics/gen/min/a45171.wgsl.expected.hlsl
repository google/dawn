void min_a45171() {
  int3 res = min(int3(0, 0, 0), int3(0, 0, 0));
}

void vertex_main() {
  min_a45171();
  return;
}

void fragment_main() {
  min_a45171();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_a45171();
  return;
}

