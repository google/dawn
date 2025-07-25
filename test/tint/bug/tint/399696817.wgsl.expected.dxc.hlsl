
groupshared int a;
void foo(uint v) {
  int4 x = unpack_s8s32(int8_t4_packed(v));
  uint4 y = unpack_u8u32(uint8_t4_packed(v));
  int v_1 = int(0);
  InterlockedOr(a, int(0), v_1);
  int z = v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

