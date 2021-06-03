void reverseBits_35fea9() {
  uint4 res = reversebits(uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  reverseBits_35fea9();
  return;
}

void fragment_main() {
  reverseBits_35fea9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_35fea9();
  return;
}

