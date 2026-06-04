#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
shared f16mat2x3 w[4];
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_16bit_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  return f16mat2x3(v_4, tint_bitcast_to_16bit_1(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2x3[4] v_7(uint start_byte_offset) {
  f16mat2x3 a[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_2((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  return a;
}
void f_inner(uint tint_local_index) {
  {
    uint v_10 = 0u;
    v_10 = tint_local_index;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      w[v_11] = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  barrier();
  w = v_7(0u);
  w[1u] = v_2(32u);
  w[1u][0u] = tint_bitcast_to_16bit_1(v.inner[0u].zw).xyz.zxy;
  uvec4 v_12 = v.inner[0u];
  w[1u][0u].x = tint_bitcast_to_16bit(v_12.z).x;
  v_1.inner = w[1u][0u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
