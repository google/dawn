RWByteAddressBuffer sb_rw : register(u0, space0);

uint tint_atomicMax(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedMax(offset, value, original_value);
  return original_value;
}


void atomicMax_51b9be() {
  uint res = tint_atomicMax(sb_rw, 0u, 1u);
}

void fragment_main() {
  atomicMax_51b9be();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicMax_51b9be();
  return;
}
