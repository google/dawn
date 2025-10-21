#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
shared f16mat4x2 w[4];
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  f16vec2 v_4 = tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return f16mat4x2(v_2, v_3, v_4, tint_bitcast_to_f16(v.inner[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}
f16mat4x2[4] v_5(uint start_byte_offset) {
  f16mat4x2 a[4] = f16mat4x2[4](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v_1((start_byte_offset + (v_7 * 16u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_8 = 0u;
    v_8 = tint_local_index;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      w[v_9] = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  barrier();
  w = v_5(0u);
  w[1u] = v_1(32u);
  w[1u][0u] = tint_bitcast_to_f16(v.inner[0u].y).yx;
  uvec4 v_10 = v.inner[0u];
  w[1u][0u].x = tint_bitcast_to_f16(v_10.y).x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
