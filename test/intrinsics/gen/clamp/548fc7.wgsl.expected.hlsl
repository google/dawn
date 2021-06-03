void clamp_548fc7() {
  uint3 res = clamp(uint3(0u, 0u, 0u), uint3(0u, 0u, 0u), uint3(0u, 0u, 0u));
}

void vertex_main() {
  clamp_548fc7();
  return;
}

void fragment_main() {
  clamp_548fc7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_548fc7();
  return;
}

