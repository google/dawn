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
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(f16mat3x2 m) {
}
void d(f16vec2 v) {
}
void e(float16_t f_1) {
}
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_16bit(v_1.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  f16vec2 v_5 = tint_bitcast_to_16bit(v_1.inner[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  return f16mat3x2(v_3, v_5, tint_bitcast_to_16bit(v_1.inner[(v_6 / 16u)][((v_6 & 15u) >> 2u)]));
}
S v_7(uint start_byte_offset) {
  uvec4 v_8 = v_1.inner[(start_byte_offset / 16u)];
  int v_9 = int(v_8[((start_byte_offset & 15u) >> 2u)]);
  f16mat3x2 v_10 = v_2((4u + start_byte_offset));
  uint v_11 = (64u + start_byte_offset);
  uvec4 v_12 = v_1.inner[(v_11 / 16u)];
  return S(v_9, v_10, int(v_12[((v_11 & 15u) >> 2u)]));
}
S[4] v_13(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a_2[v_15] = v_7((start_byte_offset + (v_15 * 128u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_13(0u));
  b(v_7(256u));
  c(v_2(260u));
  d(tint_bitcast_to_16bit(v_1.inner[0u].z).yx);
  e(tint_bitcast_to_16bit(v_1.inner[0u].z).yx.x);
}
