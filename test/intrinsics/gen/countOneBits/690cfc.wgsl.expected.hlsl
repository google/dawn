void countOneBits_690cfc() {
  uint3 res = countbits(uint3(0u, 0u, 0u));
}

void vertex_main() {
  countOneBits_690cfc();
  return;
}

void fragment_main() {
  countOneBits_690cfc();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_690cfc();
  return;
}

