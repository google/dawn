#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  ivec4 arr[4];
};

struct S_nested {
  int arr[4][3][2];
};

ivec4 src_private[4] = ivec4[4](ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0));
shared ivec4 src_workgroup[4];
layout(binding = 0, std140) uniform src_uniform_block_ubo {
  S inner;
} src_uniform;

layout(binding = 1, std430) buffer src_uniform_block_ssbo {
  S inner;
} src_storage;

layout(binding = 2, std430) buffer src_uniform_block_ssbo_1 {
  S inner;
} dst;

layout(binding = 3, std430) buffer dst_nested_block_ssbo {
  S_nested inner;
} dst_nested;

ivec4[4] ret_arr() {
  ivec4 tint_symbol_1[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  return tint_symbol_1;
}

S ret_struct_arr() {
  S tint_symbol_2 = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  return tint_symbol_2;
}

void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0), ivec4(0, 0, 0, 0));
  ivec4 tint_symbol_3[4] = ivec4[4](ivec4(1), ivec4(2), ivec4(3), ivec4(3));
  dst.inner.arr = tint_symbol_3;
  dst.inner.arr = src_param;
  dst.inner.arr = ret_arr();
  ivec4 src_let[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst.inner.arr = src_let;
  dst.inner.arr = src_function;
  dst.inner.arr = src_private;
  dst.inner.arr = src_workgroup;
  S tint_symbol = ret_struct_arr();
  dst.inner.arr = tint_symbol.arr;
  dst.inner.arr = src_uniform.inner.arr;
  dst.inner.arr = src_storage.inner.arr;
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  dst_nested.inner.arr = src_nested;
}

