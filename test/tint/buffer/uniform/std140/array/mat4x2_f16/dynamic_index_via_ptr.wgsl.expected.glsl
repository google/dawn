#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_3(uint start_byte_offset) {
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  f16vec2 v_5 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  f16vec2 v_6 = tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]);
  return f16mat4x2(v_4, v_5, v_6, tint_bitcast_to_f16(v.inner[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) & 15u) >> 2u)]));
}
f16mat4x2[4] v_7(uint start_byte_offset) {
  f16mat4x2 a[4] = f16mat4x2[4](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_3((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (min(uint(i()), 3u) * 16u);
  uint v_11 = (min(uint(i()), 3u) * 4u);
  f16mat4x2 l_a[4] = v_7(0u);
  f16mat4x2 l_a_i = v_3(v_10);
  f16vec2 l_a_i_i = tint_bitcast_to_f16(v.inner[((v_10 + v_11) / 16u)][(((v_10 + v_11) & 15u) >> 2u)]);
  uvec4 v_12 = v.inner[((v_10 + v_11) / 16u)];
  v_1.inner = (((tint_bitcast_to_f16(v_12[(((v_10 + v_11) & 15u) >> 2u)])[mix(1u, 0u, (((v_10 + v_11) % 4u) == 0u))] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
