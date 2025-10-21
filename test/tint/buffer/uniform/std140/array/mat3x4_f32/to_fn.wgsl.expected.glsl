#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[12];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat3x4 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat3x4 m) {
  return m[0u].x;
}
float c(vec4 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
mat3x4 v_3(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v_1.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v_1.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v_1.inner[((32u + start_byte_offset) / 16u)]));
}
mat3x4[4] v_4(uint start_byte_offset) {
  mat3x4 a_2[4] = mat3x4[4](mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)), mat3x4(vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a_2[v_6] = v_3((start_byte_offset + (v_6 * 48u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_7 = a(v_4(0u));
  float v_8 = (v_7 + b(v_3(48u)));
  float v_9 = (v_8 + c(uintBitsToFloat(v_1.inner[3u]).ywxz));
  v_2.inner = (v_9 + d(uintBitsToFloat(v_1.inner[3u]).ywxz.x));
}
