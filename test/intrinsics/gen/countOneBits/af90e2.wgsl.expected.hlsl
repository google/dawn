void countOneBits_af90e2() {
  int2 res = countbits(int2(0, 0));
}

void vertex_main() {
  countOneBits_af90e2();
  return;
}

void fragment_main() {
  countOneBits_af90e2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  countOneBits_af90e2();
  return;
}

