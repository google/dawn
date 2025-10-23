struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int a;
void foo(uint v) {
  int4 x = unpack_s8s32(int8_t4_packed(v));
  uint4 y = unpack_u8u32(uint8_t4_packed(v));
  int v_1 = int(0);
  InterlockedOr(a, int(0), v_1);
  int z = v_1;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_2 = int(0);
    InterlockedExchange(a, int(0), v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  foo(1u);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

