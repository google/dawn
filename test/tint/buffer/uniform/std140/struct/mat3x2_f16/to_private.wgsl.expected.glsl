#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat3x2 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
S p[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  return f16mat3x2(v_2, v_4, tint_bitcast_to_16bit(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}
S v_6(uint start_byte_offset) {
  uvec4 v_7 = v.inner[(start_byte_offset / 16u)];
  int v_8 = int(v_7[((start_byte_offset & 15u) >> 2u)]);
  f16mat3x2 v_9 = v_1((4u + start_byte_offset));
  uint v_10 = (64u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return S(v_8, v_9, int(v_11[((v_10 & 15u) >> 2u)]));
}
S[4] v_12(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_6((start_byte_offset + (v_14 * 128u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_12(0u);
  p[1u] = v_6(256u);
  p[3u].m = v_1(260u);
  p[1u].m[0u] = tint_bitcast_to_16bit(v.inner[0u].z).yx;
}
