#version 310 es


struct S {
  ivec4 arr[4];
};

ivec4 src_private[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
shared ivec4 src_workgroup[4];
layout(binding = 0, std140)
uniform src_uniform_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer src_storage_block_1_ssbo {
  S inner;
} v_1;
shared ivec4 dst[4];
shared int dst_nested[4][3][2];
ivec4[4] ret_arr() {
  return ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
}
S ret_struct_arr() {
  return S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
}
ivec4[4] v_2(uint start_byte_offset) {
  ivec4 a[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = ivec4(v.inner[((start_byte_offset + (v_3 * 16u)) / 16u)]);
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst = ivec4[4](ivec4(1), ivec4(2), ivec4(3), ivec4(3));
  dst = src_param;
  dst = ret_arr();
  ivec4 src_let[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst = src_let;
  dst = src_function;
  dst = src_private;
  dst = src_workgroup;
  dst = ret_struct_arr().arr;
  dst = v_2(0u);
  dst = v_1.inner.arr;
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  dst_nested = src_nested;
}
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      src_workgroup[v_4] = ivec4(0);
      dst[v_4] = ivec4(0);
      {
        idx = (idx + 1u);
      }
    }
  }
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 24u)) {
        break;
      }
      dst_nested[(v_5 / 6u)][((v_5 / 2u) % 3u)][(v_5 % 2u)] = 0;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  ivec4 val[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  foo(val);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
