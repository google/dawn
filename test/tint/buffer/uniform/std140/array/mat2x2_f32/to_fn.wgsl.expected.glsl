#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat2 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat2 m) {
  return m[0u].x;
}
float c(vec2 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
mat2 v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_6 = (8u + start_byte_offset);
  uvec4 v_7 = v_1.inner[(v_6 / 16u)];
  return mat2(v_5, uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2((((v_6 & 15u) >> 2u) == 2u)))));
}
mat2[4] v_8(uint start_byte_offset) {
  mat2 a_2[4] = mat2[4](mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)), mat2(vec2(0.0f), vec2(0.0f)));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_2[v_10] = v_3((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_11 = a(v_8(0u));
  float v_12 = (v_11 + b(v_3(16u)));
  float v_13 = (v_12 + c(uintBitsToFloat(v_1.inner[1u].xy).yx));
  v_2.inner = (v_13 + d(uintBitsToFloat(v_1.inner[1u].xy).yx.x));
}
