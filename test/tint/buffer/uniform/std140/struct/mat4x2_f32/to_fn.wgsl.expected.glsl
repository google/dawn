#version 310 es


struct S {
  int before;
  mat4x2 m;
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
void c(mat4x2 m) {
}
void d(vec2 v) {
}
void e(float f_1) {
}
mat4x2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  vec2 v_7 = uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v_1.inner[(v_8 / 16u)];
  vec2 v_10 = uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u))));
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v_1.inner[(v_11 / 16u)];
  return mat4x2(v_4, v_7, v_10, uintBitsToFloat(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))));
}
S v_13(uint start_byte_offset) {
  uvec4 v_14 = v_1.inner[(start_byte_offset / 16u)];
  int v_15 = int(v_14[((start_byte_offset & 15u) >> 2u)]);
  mat4x2 v_16 = v_2((8u + start_byte_offset));
  uint v_17 = (64u + start_byte_offset);
  uvec4 v_18 = v_1.inner[(v_17 / 16u)];
  return S(v_15, v_16, int(v_18[((v_17 & 15u) >> 2u)]));
}
S[4] v_19(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0), S(0, mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), 0));
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 4u)) {
        break;
      }
      a_2[v_21] = v_13((start_byte_offset + (v_21 * 128u)));
      {
        v_20 = (v_21 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_19(0u));
  b(v_13(256u));
  c(v_2(264u));
  d(uintBitsToFloat(v_1.inner[1u].xy).yx);
  e(uintBitsToFloat(v_1.inner[1u].xy).yx.x);
}
