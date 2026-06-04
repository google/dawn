#version 310 es


struct S {
  int before;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  mat2x3 m;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  int after;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
  uint tint_pad_18;
  uint tint_pad_19;
  uint tint_pad_20;
  uint tint_pad_21;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner[4];
} v_1;
void tint_store_and_preserve_padding_2(uint target_indices[1], mat2x3 value_param) {
  v_1.inner[target_indices[0u]].m[0u] = value_param[0u];
  v_1.inner[target_indices[0u]].m[1u] = value_param[1u];
}
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
void tint_store_and_preserve_padding_1(uint target_indices[1], S value_param) {
  v_1.inner[target_indices[0u]].before = value_param.before;
  tint_store_and_preserve_padding_2(uint[1](target_indices[0u]), value_param.m);
  v_1.inner[target_indices[0u]].after = value_param.after;
}
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  int v_5 = int(v_4[((start_byte_offset & 15u) >> 2u)]);
  mat2x3 v_6 = v_2((16u + start_byte_offset));
  uint v_7 = (64u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return S(v_5, 0u, 0u, 0u, v_6, 0u, 0u, 0u, 0u, int(v_8[((v_7 & 15u) >> 2u)]), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
}
void tint_store_and_preserve_padding(S value_param[4]) {
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_10), value_param[v_10]);
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
}
S[4] v_11(uint start_byte_offset) {
  S a[4] = S[4](S(0, 0u, 0u, 0u, mat2x3(vec3(0.0f), vec3(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(vec3(0.0f), vec3(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(vec3(0.0f), vec3(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x3(vec3(0.0f), vec3(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_3((start_byte_offset + (v_13 * 128u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_11(0u));
  S v_14 = v_3(256u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_14);
  mat2x3 v_15 = v_2(272u);
  tint_store_and_preserve_padding_2(uint[1](3u), v_15);
  v_1.inner[1u].m[0u] = uintBitsToFloat(v.inner[2u].xyz).zxy;
}
