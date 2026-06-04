#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
shared f16mat2x4 w[4];
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_16bit_1(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  return f16mat2x4(v_3, tint_bitcast_to_16bit_1(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u)))));
}
f16mat2x4[4] v_6(uint start_byte_offset) {
  f16mat2x4 a[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_1((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_9 = 0u;
    v_9 = tint_local_index;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      w[v_10] = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  barrier();
  w = v_6(0u);
  w[1u] = v_1(32u);
  w[1u][0u] = tint_bitcast_to_16bit_1(v.inner[0u].zw).ywxz;
  uvec4 v_11 = v.inner[0u];
  w[1u][0u].x = tint_bitcast_to_16bit(v_11.z).x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
