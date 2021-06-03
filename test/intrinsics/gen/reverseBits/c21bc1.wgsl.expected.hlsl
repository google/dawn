void reverseBits_c21bc1() {
  int3 res = reversebits(int3(0, 0, 0));
}

void vertex_main() {
  reverseBits_c21bc1();
  return;
}

void fragment_main() {
  reverseBits_c21bc1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_c21bc1();
  return;
}

