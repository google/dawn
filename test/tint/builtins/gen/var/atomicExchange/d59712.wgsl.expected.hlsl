RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicExchange(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedExchange(offset, value, original_value);
  return original_value;
}


void atomicExchange_d59712() {
  uint arg_1 = 1u;
  uint res = tint_atomicExchange(sb_rw, 0u, arg_1);
}

void fragment_main() {
  atomicExchange_d59712();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicExchange_d59712();
  return;
}
