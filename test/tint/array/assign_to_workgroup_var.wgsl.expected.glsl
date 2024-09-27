#version 310 es

uint tint_div(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % mix(rhs, 1u, (rhs == 0u)));
}

shared ivec4 dst[4];
shared ivec4 src_workgroup[4];
shared int dst_nested[4][3][2];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      dst[i] = ivec4(0);
      src_workgroup[i] = ivec4(0);
    }
  }
  {
    for(uint idx_1 = local_idx; (idx_1 < 24u); idx_1 = (idx_1 + 1u)) {
      uint i_1 = tint_div(idx_1, 6u);
      uint i_2 = tint_div(tint_mod(idx_1, 6u), 2u);
      uint i_3 = tint_mod(idx_1, 2u);
      dst_nested[i_1][i_2][i_3] = 0;
    }
  }
  barrier();
}

struct S {
  ivec4 arr[4];
};

ivec4 src_private[4] = ivec4[4](ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0));
layout(binding = 0, std140) uniform src_uniform_block_ubo {
  S inner;
} src_uniform;

layout(binding = 1, std430) buffer src_uniform_block_ssbo {
  S inner;
} src_storage;

ivec4[4] ret_arr() {
  ivec4 tint_symbol_2[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  return tint_symbol_2;
}

S ret_struct_arr() {
  S tint_symbol_3 = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  return tint_symbol_3;
}

void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0));
  ivec4 tint_symbol_4[4] = ivec4[4](ivec4(1), ivec4(2), ivec4(3), ivec4(3));
  dst = tint_symbol_4;
  dst = src_param;
  dst = ret_arr();
  ivec4 src_let[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst = src_let;
  dst = src_function;
  dst = src_private;
  dst = src_workgroup;
  S tint_symbol_1 = ret_struct_arr();
  dst = tint_symbol_1.arr;
  dst = src_uniform.inner.arr;
  dst = src_storage.inner.arr;
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  dst_nested = src_nested;
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  ivec4 val[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  foo(val);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
