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
f16mat2x3 p[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_f16_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_4, tint_bitcast_to_f16_1(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2x3[4] v_6(uint start_byte_offset) {
  f16mat2x3 a[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_6(0u);
  p[1u] = v_2(32u);
  p[1u][0u] = tint_bitcast_to_f16_1(v.inner[0u].zw).xyz.zxy;
  uvec4 v_9 = v.inner[0u];
  p[1u][0u].x = tint_bitcast_to_f16(v_9.z).x;
  v_1.inner = p[1u][0u].x;
}
