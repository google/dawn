void countOneBits_ae44f9() {
  uint res = countbits(1u);
}

void vertex_main() {
  countOneBits_ae44f9();
  return;
}

void fragment_main() {
  countOneBits_ae44f9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_ae44f9();
  return;
}

