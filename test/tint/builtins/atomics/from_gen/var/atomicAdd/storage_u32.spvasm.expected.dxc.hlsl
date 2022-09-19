RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicAdd(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void atomicAdd_8a199a() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  const uint x_18 = arg_1;
  const uint x_13 = tint_atomicAdd(sb_rw, 0u, x_18);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicAdd_8a199a();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicAdd_8a199a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
