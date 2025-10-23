#version 310 es

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float inner;
} v_2;
float a(mat4x2 a_1[4]) {
  return a_1[0u][0u].x;
}
float b(mat4x2 m) {
  return m[0u].x;
}
float c(vec2 v) {
  return v.x;
}
float d(float f_1) {
  return f_1;
}
mat4x2 v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_5 = uintBitsToFloat(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_6 = v_1.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_7 = uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_8 = v_1.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_10 = v_1.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_5, v_7, v_9, uintBitsToFloat(mix(v_10.xy, v_10.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
mat4x2[4] v_11(uint start_byte_offset) {
  mat4x2 a_2[4] = mat4x2[4](mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)), mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f)));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a_2[v_13] = v_3((start_byte_offset + (v_13 * 32u)));
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v_14 = a(v_11(0u));
  float v_15 = (v_14 + b(v_3(32u)));
  float v_16 = (v_15 + c(uintBitsToFloat(v_1.inner[2u].xy).yx));
  v_2.inner = (v_16 + d(uintBitsToFloat(v_1.inner[2u].xy).yx.x));
}
