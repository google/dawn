void countOneBits_0f7980() {
  int4 res = countbits(int4(0, 0, 0, 0));
}

void vertex_main() {
  countOneBits_0f7980();
  return;
}

void fragment_main() {
  countOneBits_0f7980();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_0f7980();
  return;
}

