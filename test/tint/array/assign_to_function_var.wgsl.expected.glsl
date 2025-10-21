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
ivec4[4] ret_arr() {
  return ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
}
S ret_struct_arr() {
  return S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
}
ivec4[4] v_2(uint start_byte_offset) {
  ivec4 a[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = ivec4(v.inner[((start_byte_offset + (v_4 * 16u)) / 16u)]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  return a;
}
void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  ivec4 dst[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
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
  int dst_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  dst_nested = src_nested;
}
void main_inner(uint tint_local_index) {
  {
    uint v_5 = 0u;
    v_5 = tint_local_index;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      src_workgroup[v_6] = ivec4(0);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
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
