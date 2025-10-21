#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat2x3 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v;
S p[4] = S[4](S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0));
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_3 = tint_bitcast_to_f16(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u)))).xyz;
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_3, tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))).xyz);
}
S v_5(uint start_byte_offset) {
  uvec4 v_6 = v.inner[(start_byte_offset / 16u)];
  int v_7 = int(v_6[((start_byte_offset % 16u) / 4u)]);
  f16mat2x3 v_8 = v_1((8u + start_byte_offset));
  uvec4 v_9 = v.inner[((64u + start_byte_offset) / 16u)];
  return S(v_7, v_8, int(v_9[(((64u + start_byte_offset) % 16u) / 4u)]));
}
S[4] v_10(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), 0));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_5((start_byte_offset + (v_12 * 128u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_10(0u);
  p[1u] = v_5(256u);
  p[3u].m = v_1(264u);
  p[1u].m[0u] = tint_bitcast_to_f16(v.inner[1u].xy).xyz.zxy;
}
