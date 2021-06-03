void countOneBits_fd88b2() {
  int res = countbits(1);
}

void vertex_main() {
  countOneBits_fd88b2();
  return;
}

void fragment_main() {
  countOneBits_fd88b2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_fd88b2();
  return;
}

