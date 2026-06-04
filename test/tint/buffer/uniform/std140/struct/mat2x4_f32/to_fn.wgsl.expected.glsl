#version 310 es


struct S {
  int before;
  mat2x4 m;
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
void c(mat2x4 m) {
}
void d(vec4 v) {
}
void e(float f_1) {
}
mat2x4 v_2(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]));
}
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  int v_5 = int(v_4[((start_byte_offset & 15u) >> 2u)]);
  mat2x4 v_6 = v_2((16u + start_byte_offset));
  uint v_7 = (64u + start_byte_offset);
  uvec4 v_8 = v_1.inner[(v_7 / 16u)];
  return S(v_5, v_6, int(v_8[((v_7 & 15u) >> 2u)]));
}
S[4] v_9(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0));
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a_2[v_11] = v_3((start_byte_offset + (v_11 * 128u)));
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_9(0u));
  b(v_3(256u));
  c(v_2(272u));
  d(uintBitsToFloat(v_1.inner[2u]).ywxz);
  e(uintBitsToFloat(v_1.inner[2u]).ywxz.x);
}
