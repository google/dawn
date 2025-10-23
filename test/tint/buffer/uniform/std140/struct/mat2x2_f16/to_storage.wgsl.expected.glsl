#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat2 m;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  int after;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
  uint tint_pad_18;
  uint tint_pad_19;
  uint tint_pad_20;
  uint tint_pad_21;
  uint tint_pad_22;
  uint tint_pad_23;
  uint tint_pad_24;
  uint tint_pad_25;
  uint tint_pad_26;
  uint tint_pad_27;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner[4];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return f16mat2(v_3, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}
void tint_store_and_preserve_padding_1(uint target_indices[1], S value_param) {
  v_1.inner[target_indices[0u]].before = value_param.before;
  v_1.inner[target_indices[0u]].m = value_param.m;
  v_1.inner[target_indices[0u]].after = value_param.after;
}
S v_4(uint start_byte_offset) {
  uvec4 v_5 = v.inner[(start_byte_offset / 16u)];
  int v_6 = int(v_5[((start_byte_offset & 15u) >> 2u)]);
  f16mat2 v_7 = v_2((4u + start_byte_offset));
  uvec4 v_8 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_6, v_7, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, int(v_8[(((64u + start_byte_offset) & 15u) >> 2u)]), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
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
      continue;
    }
  }
}
S[4] v_11(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat2(f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_4((start_byte_offset + (v_13 * 128u)));
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_11(0u));
  S v_14 = v_4(256u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_14);
  v_1.inner[3u].m = v_2(260u);
  v_1.inner[1u].m[0u] = tint_bitcast_to_f16(v.inner[0u].z).yx;
}
