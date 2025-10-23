#version 310 es


struct S {
  int before;
  mat4x3 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[48];
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(mat4x3 m) {
}
void d(vec3 v) {
}
void e(float f_1) {
}
mat4x3 v_2(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v_1.inner[((48u + start_byte_offset) / 16u)].xyz));
}
S v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  int v_5 = int(v_4[((start_byte_offset & 15u) >> 2u)]);
  mat4x3 v_6 = v_2((16u + start_byte_offset));
  uvec4 v_7 = v_1.inner[((128u + start_byte_offset) / 16u)];
  return S(v_5, v_6, int(v_7[(((128u + start_byte_offset) & 15u) >> 2u)]));
}
S[4] v_8(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0), S(0, mat4x3(vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_2[v_10] = v_3((start_byte_offset + (v_10 * 192u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_8(0u));
  b(v_3(384u));
  c(v_2(400u));
  d(uintBitsToFloat(v_1.inner[2u].xyz).zxy);
  e(uintBitsToFloat(v_1.inner[2u].xyz).zxy.x);
}
