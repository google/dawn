void countOneBits_0d0e46() {
  uint4 res = countbits(uint4(0u, 0u, 0u, 0u));
}

void vertex_main() {
  countOneBits_0d0e46();
  return;
}

void fragment_main() {
  countOneBits_0d0e46();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_0d0e46();
  return;
}

