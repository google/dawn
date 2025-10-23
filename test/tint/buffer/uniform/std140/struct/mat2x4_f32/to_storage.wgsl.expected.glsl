#version 310 es


struct S {
  int before;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  mat2x4 m;
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
mat2x4 v_2(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
void tint_store_and_preserve_padding_1(uint target_indices[1], S value_param) {
  v_1.inner[target_indices[0u]].before = value_param.before;
  v_1.inner[target_indices[0u]].m = value_param.m;
  v_1.inner[target_indices[0u]].after = value_param.after;
}
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  int v_5 = int(v_4[((start_byte_offset & 15u) >> 2u)]);
  mat2x4 v_6 = v_2((16u + start_byte_offset));
  uvec4 v_7 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_5, 0u, 0u, 0u, v_6, 0u, 0u, 0u, 0u, int(v_7[(((64u + start_byte_offset) & 15u) >> 2u)]), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
}
void tint_store_and_preserve_padding(S value_param[4]) {
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_9), value_param[v_9]);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
}
S[4] v_10(uint start_byte_offset) {
  S a[4] = S[4](S(0, 0u, 0u, 0u, mat2x4(vec4(0.0f), vec4(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x4(vec4(0.0f), vec4(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x4(vec4(0.0f), vec4(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, 0u, 0u, 0u, mat2x4(vec4(0.0f), vec4(0.0f)), 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_3((start_byte_offset + (v_12 * 128u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_10(0u));
  S v_13 = v_3(256u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_13);
  v_1.inner[3u].m = v_2(272u);
  v_1.inner[1u].m[0u] = uintBitsToFloat(v.inner[2u]).ywxz;
}
