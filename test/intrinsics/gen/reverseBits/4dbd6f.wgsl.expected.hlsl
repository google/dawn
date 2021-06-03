void reverseBits_4dbd6f() {
  int4 res = reversebits(int4(0, 0, 0, 0));
}

void vertex_main() {
  reverseBits_4dbd6f();
  return;
}

void fragment_main() {
  reverseBits_4dbd6f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reverseBits_4dbd6f();
  return;
}

