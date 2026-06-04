#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat3x4 m;
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
void c(f16mat3x4 m) {
}
void d(f16vec4 v) {
}
void e(float16_t f_1) {
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  f16vec4 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v_1.inner[(v_8 / 16u)];
  return f16mat3x4(v_4, v_7, tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))));
}
S v_10(uint start_byte_offset) {
  uvec4 v_11 = v_1.inner[(start_byte_offset / 16u)];
  int v_12 = int(v_11[((start_byte_offset & 15u) >> 2u)]);
  f16mat3x4 v_13 = v_2((8u + start_byte_offset));
  uint v_14 = (64u + start_byte_offset);
  uvec4 v_15 = v_1.inner[(v_14 / 16u)];
  return S(v_12, v_13, int(v_15[((v_14 & 15u) >> 2u)]));
}
S[4] v_16(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0));
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      a_2[v_18] = v_10((start_byte_offset + (v_18 * 128u)));
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_16(0u));
  b(v_10(256u));
  c(v_2(264u));
  d(tint_bitcast_to_16bit(v_1.inner[1u].xy).ywxz);
  e(tint_bitcast_to_16bit(v_1.inner[1u].xy).ywxz.x);
}
