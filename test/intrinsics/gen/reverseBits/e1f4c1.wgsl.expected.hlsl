void reverseBits_e1f4c1() {
  uint2 res = reversebits(uint2(0u, 0u));
}

void vertex_main() {
  reverseBits_e1f4c1();
  return;
}

void fragment_main() {
  reverseBits_e1f4c1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_e1f4c1();
  return;
}

