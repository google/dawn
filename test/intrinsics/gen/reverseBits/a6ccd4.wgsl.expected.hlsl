void reverseBits_a6ccd4() {
  uint3 res = reversebits(uint3(0u, 0u, 0u));
}

void vertex_main() {
  reverseBits_a6ccd4();
  return;
}

void fragment_main() {
  reverseBits_a6ccd4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_a6ccd4();
  return;
}

