
RWByteAddressBuffer sb_rw : register(u0);
void atomicLoad_0806ad() {
  int res = 0;
  int v = 0;
  sb_rw.InterlockedOr(int(0u), 0, v);
  int x_9 = v;
  res = x_9;
}

void fragment_main_1() {
  atomicLoad_0806ad();
}

void fragment_main() {
  fragment_main_1();
}

void compute_main_1() {
  atomicLoad_0806ad();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

