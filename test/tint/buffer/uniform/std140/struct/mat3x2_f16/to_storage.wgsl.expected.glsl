#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat3x2 m;
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
  int after;
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
  uint tint_pad_22;
  uint tint_pad_23;
  uint tint_pad_24;
  uint tint_pad_25;
  uint tint_pad_26;
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
f16mat3x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return f16mat3x2(v_3, v_4, tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}
void tint_store_and_preserve_padding_1(uint target_indices[1], S value_param) {
  v_1.inner[target_indices[0u]].before = value_param.before;
  v_1.inner[target_indices[0u]].m = value_param.m;
  v_1.inner[target_indices[0u]].after = value_param.after;
}
S v_5(uint start_byte_offset) {
  uvec4 v_6 = v.inner[(start_byte_offset / 16u)];
  int v_7 = int(v_6[((start_byte_offset % 16u) / 4u)]);
  f16mat3x2 v_8 = v_2((4u + start_byte_offset));
  uvec4 v_9 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_7, v_8, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, int(v_9[(((64u + start_byte_offset) % 16u) / 4u)]), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
}
void tint_store_and_preserve_padding(S value_param[4]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_11), value_param[v_11]);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
}
S[4] v_12(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_5((start_byte_offset + (v_14 * 128u)));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_12(0u));
  S v_15 = v_5(256u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_15);
  v_1.inner[3u].m = v_2(260u);
  v_1.inner[1u].m[0u] = tint_bitcast_to_f16(v.inner[0u].z).yx;
}
