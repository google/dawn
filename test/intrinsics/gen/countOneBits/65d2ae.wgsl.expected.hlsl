void countOneBits_65d2ae() {
  int3 res = countbits(int3(0, 0, 0));
}

void vertex_main() {
  countOneBits_65d2ae();
  return;
}

void fragment_main() {
  countOneBits_65d2ae();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_65d2ae();
  return;
}

