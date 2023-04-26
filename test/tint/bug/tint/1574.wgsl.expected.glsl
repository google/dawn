#version 310 es

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


layout(binding = 0, std430) buffer a_u32_block_ssbo {
  uint inner;
} a_u32;

layout(binding = 1, std430) buffer a_i32_block_ssbo {
  int inner;
} a_i32;

shared uint b_u32;
shared int b_i32;
void tint_symbol(uint local_invocation_index) {
  if ((local_invocation_index < 1u)) {
    atomicExchange(b_u32, 0u);
    atomicExchange(b_i32, 0);
  }
  barrier();
  {
    uint value = 42u;
    atomic_compare_exchange_result_u32 atomic_compare_result;
    atomic_compare_result.old_value = atomicCompSwap(a_u32.inner, 0u, value);
    atomic_compare_result.exchanged = atomic_compare_result.old_value == 0u;
    atomic_compare_exchange_result_u32 r1 = atomic_compare_result;
    atomic_compare_exchange_result_u32 atomic_compare_result_1;
    atomic_compare_result_1.old_value = atomicCompSwap(a_u32.inner, 0u, value);
    atomic_compare_result_1.exchanged = atomic_compare_result_1.old_value == 0u;
    atomic_compare_exchange_result_u32 r2 = atomic_compare_result_1;
    atomic_compare_exchange_result_u32 atomic_compare_result_2;
    atomic_compare_result_2.old_value = atomicCompSwap(a_u32.inner, 0u, value);
    atomic_compare_result_2.exchanged = atomic_compare_result_2.old_value == 0u;
    atomic_compare_exchange_result_u32 r3 = atomic_compare_result_2;
  }
  {
    int value = 42;
    atomic_compare_exchange_result_i32 atomic_compare_result_3;
    atomic_compare_result_3.old_value = atomicCompSwap(a_i32.inner, 0, value);
    atomic_compare_result_3.exchanged = atomic_compare_result_3.old_value == 0;
    atomic_compare_exchange_result_i32 r1 = atomic_compare_result_3;
    atomic_compare_exchange_result_i32 atomic_compare_result_4;
    atomic_compare_result_4.old_value = atomicCompSwap(a_i32.inner, 0, value);
    atomic_compare_result_4.exchanged = atomic_compare_result_4.old_value == 0;
    atomic_compare_exchange_result_i32 r2 = atomic_compare_result_4;
    atomic_compare_exchange_result_i32 atomic_compare_result_5;
    atomic_compare_result_5.old_value = atomicCompSwap(a_i32.inner, 0, value);
    atomic_compare_result_5.exchanged = atomic_compare_result_5.old_value == 0;
    atomic_compare_exchange_result_i32 r3 = atomic_compare_result_5;
  }
  {
    uint value = 42u;
    atomic_compare_exchange_result_u32 atomic_compare_result_6;
    atomic_compare_result_6.old_value = atomicCompSwap(b_u32, 0u, value);
    atomic_compare_result_6.exchanged = atomic_compare_result_6.old_value == 0u;
    atomic_compare_exchange_result_u32 r1 = atomic_compare_result_6;
    atomic_compare_exchange_result_u32 atomic_compare_result_7;
    atomic_compare_result_7.old_value = atomicCompSwap(b_u32, 0u, value);
    atomic_compare_result_7.exchanged = atomic_compare_result_7.old_value == 0u;
    atomic_compare_exchange_result_u32 r2 = atomic_compare_result_7;
    atomic_compare_exchange_result_u32 atomic_compare_result_8;
    atomic_compare_result_8.old_value = atomicCompSwap(b_u32, 0u, value);
    atomic_compare_result_8.exchanged = atomic_compare_result_8.old_value == 0u;
    atomic_compare_exchange_result_u32 r3 = atomic_compare_result_8;
  }
  {
    int value = 42;
    atomic_compare_exchange_result_i32 atomic_compare_result_9;
    atomic_compare_result_9.old_value = atomicCompSwap(b_i32, 0, value);
    atomic_compare_result_9.exchanged = atomic_compare_result_9.old_value == 0;
    atomic_compare_exchange_result_i32 r1 = atomic_compare_result_9;
    atomic_compare_exchange_result_i32 atomic_compare_result_10;
    atomic_compare_result_10.old_value = atomicCompSwap(b_i32, 0, value);
    atomic_compare_result_10.exchanged = atomic_compare_result_10.old_value == 0;
    atomic_compare_exchange_result_i32 r2 = atomic_compare_result_10;
    atomic_compare_exchange_result_i32 atomic_compare_result_11;
    atomic_compare_result_11.old_value = atomicCompSwap(b_i32, 0, value);
    atomic_compare_result_11.exchanged = atomic_compare_result_11.old_value == 0;
    atomic_compare_exchange_result_i32 r3 = atomic_compare_result_11;
  }
}

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
