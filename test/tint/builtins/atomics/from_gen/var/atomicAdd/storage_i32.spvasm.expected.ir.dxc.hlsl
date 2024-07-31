
RWByteAddressBuffer sb_rw : register(u0);
void atomicAdd_d32fe4() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int v = 0;
  sb_rw.InterlockedAdd(int(0u), x_20, v);
  int x_13 = v;
  res = x_13;
}

void fragment_main_1() {
  atomicAdd_d32fe4();
}

void fragment_main() {
  fragment_main_1();
}

void compute_main_1() {
  atomicAdd_d32fe4();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

