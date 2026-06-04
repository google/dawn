#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat4x2 m;
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
void c(f16mat4x2 m) {
}
void d(f16vec2 v) {
}
void e(float16_t f_1) {
}
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_16bit(v_1.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  f16vec2 v_5 = tint_bitcast_to_16bit(v_1.inner[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  f16vec2 v_7 = tint_bitcast_to_16bit(v_1.inner[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return f16mat4x2(v_3, v_5, v_7, tint_bitcast_to_16bit(v_1.inner[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}
S v_9(uint start_byte_offset) {
  uvec4 v_10 = v_1.inner[(start_byte_offset / 16u)];
  int v_11 = int(v_10[((start_byte_offset & 15u) >> 2u)]);
  f16mat4x2 v_12 = v_2((4u + start_byte_offset));
  uint v_13 = (64u + start_byte_offset);
  uvec4 v_14 = v_1.inner[(v_13 / 16u)];
  return S(v_11, v_12, int(v_14[((v_13 & 15u) >> 2u)]));
}
S[4] v_15(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      a_2[v_17] = v_9((start_byte_offset + (v_17 * 128u)));
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_15(0u));
  b(v_9(256u));
  c(v_2(260u));
  d(tint_bitcast_to_16bit(v_1.inner[0u].z).yx);
  e(tint_bitcast_to_16bit(v_1.inner[0u].z).yx.x);
}
