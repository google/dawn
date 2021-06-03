void min_82b28f() {
  uint2 res = min(uint2(0u, 0u), uint2(0u, 0u));
}

void vertex_main() {
  min_82b28f();
  return;
}

void fragment_main() {
  min_82b28f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  min_82b28f();
  return;
}

