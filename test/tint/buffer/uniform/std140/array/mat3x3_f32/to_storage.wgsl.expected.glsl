#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[12];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat3 inner[4];
} v_1;
void tint_store_and_preserve_padding_1(uint target_indices[1], mat3 value_param) {
  v_1.inner[target_indices[0u]][0u] = value_param[0u];
  v_1.inner[target_indices[0u]][1u] = value_param[1u];
  v_1.inner[target_indices[0u]][2u] = value_param[2u];
}
mat3 v_2(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
void tint_store_and_preserve_padding(mat3 value_param[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_3), value_param[v_3]);
      {
        idx = (idx + 1u);
      }
    }
  }
}
mat3[4] v_4(uint start_byte_offset) {
  mat3 a[4] = mat3[4](mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)), mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v_2((start_byte_offset + (v_5 * 48u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_4(0u));
  tint_store_and_preserve_padding_1(uint[1](1u), v_2(96u));
  v_1.inner[1][0] = uintBitsToFloat(v.inner[1u].xyz).zxy;
  uvec4 v_6 = v.inner[1u];
  v_1.inner[1][0].x = uintBitsToFloat(v_6.x);
}
