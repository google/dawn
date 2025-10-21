#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat2x3 inner[4];
} v_1;
void tint_store_and_preserve_padding_1(uint target_indices[1], mat2x3 value_param) {
  v_1.inner[target_indices[0u]][0u] = value_param[0u];
  v_1.inner[target_indices[0u]][1u] = value_param[1u];
}
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
void tint_store_and_preserve_padding(mat2x3 value_param[4]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_4), value_param[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}
mat2x3[4] v_5(uint start_byte_offset) {
  mat2x3 a[4] = mat2x3[4](mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)), mat2x3(vec3(0.0f), vec3(0.0f)));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v_2((start_byte_offset + (v_7 * 32u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_5(0u));
  mat2x3 v_8 = v_2(64u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_8);
  v_1.inner[1u][0u] = uintBitsToFloat(v.inner[1u].xyz).zxy;
  uvec4 v_9 = v.inner[1u];
  v_1.inner[1u][0u].x = uintBitsToFloat(v_9.x);
}
