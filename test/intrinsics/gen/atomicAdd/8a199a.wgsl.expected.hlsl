uint atomicAdd_1(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}

RWByteAddressBuffer sb_rw : register(u0, space0);

void atomicAdd_8a199a() {
  uint res = atomicAdd_1(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicAdd_8a199a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicAdd_8a199a();
  return;
}
