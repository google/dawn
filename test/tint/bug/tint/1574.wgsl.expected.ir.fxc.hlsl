struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer a_u32 : register(u0);
RWByteAddressBuffer a_i32 : register(u1);
groupshared uint b_u32;
groupshared int b_i32;
void main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    uint v = 0u;
    InterlockedExchange(b_u32, 0u, v);
    int v_1 = int(0);
    InterlockedExchange(b_i32, int(0), v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  uint value = 42u;
  uint v_2 = value;
  uint v_3 = 0u;
  a_u32.InterlockedCompareExchange(uint(0u), 0u, v_2, v_3);
  uint v_4 = v_3;
  atomic_compare_exchange_result_u32 r1 = {v_4, (v_4 == 0u)};
  uint v_5 = value;
  uint v_6 = 0u;
  a_u32.InterlockedCompareExchange(uint(0u), 0u, v_5, v_6);
  uint v_7 = v_6;
  atomic_compare_exchange_result_u32 r2 = {v_7, (v_7 == 0u)};
  uint v_8 = value;
  uint v_9 = 0u;
  a_u32.InterlockedCompareExchange(uint(0u), 0u, v_8, v_9);
  uint v_10 = v_9;
  atomic_compare_exchange_result_u32 r3 = {v_10, (v_10 == 0u)};
  int value_1 = int(42);
  int v_11 = value_1;
  int v_12 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_11, v_12);
  int v_13 = v_12;
  atomic_compare_exchange_result_i32 r1_1 = {v_13, (v_13 == int(0))};
  int v_14 = value_1;
  int v_15 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_14, v_15);
  int v_16 = v_15;
  atomic_compare_exchange_result_i32 r2_1 = {v_16, (v_16 == int(0))};
  int v_17 = value_1;
  int v_18 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_17, v_18);
  int v_19 = v_18;
  atomic_compare_exchange_result_i32 r3_1 = {v_19, (v_19 == int(0))};
  uint value_2 = 42u;
  uint v_20 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_20);
  uint v_21 = v_20;
  atomic_compare_exchange_result_u32 r1_2 = {v_21, (v_21 == 0u)};
  uint v_22 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_22);
  uint v_23 = v_22;
  atomic_compare_exchange_result_u32 r2_2 = {v_23, (v_23 == 0u)};
  uint v_24 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_24);
  uint v_25 = v_24;
  atomic_compare_exchange_result_u32 r3_2 = {v_25, (v_25 == 0u)};
  int value_3 = int(42);
  int v_26 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_26);
  int v_27 = v_26;
  atomic_compare_exchange_result_i32 r1_3 = {v_27, (v_27 == int(0))};
  int v_28 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_28);
  int v_29 = v_28;
  atomic_compare_exchange_result_i32 r2_3 = {v_29, (v_29 == int(0))};
  int v_30 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_30);
  int v_31 = v_30;
  atomic_compare_exchange_result_i32 r3_3 = {v_31, (v_31 == int(0))};
}

[numthreads(16, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

