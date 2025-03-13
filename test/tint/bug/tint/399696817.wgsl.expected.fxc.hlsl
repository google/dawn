SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int a;

void foo(uint tint_symbol) {
  int4 x = unpack_s8s32(int8_t4_packed(tint_symbol));
  uint4 y = unpack_u8u32(uint8_t4_packed(tint_symbol));
  int atomic_result = 0;
  InterlockedOr(a, 0, atomic_result);
  int z = atomic_result;
}
FXC validation failure:
<scrubbed_path>(9,25-51): error X3004: undeclared identifier 'int8_t4_packed'


tint executable returned error: exit status 1