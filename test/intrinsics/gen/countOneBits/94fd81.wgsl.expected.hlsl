void countOneBits_94fd81() {
  uint2 res = countbits(uint2(0u, 0u));
}

void vertex_main() {
  countOneBits_94fd81();
  return;
}

void fragment_main() {
  countOneBits_94fd81();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_94fd81();
  return;
}

