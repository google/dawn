#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat3 m;
  int after;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[32];
} v_1;
void a(S a_1[4]) {
}
void b(S s) {
}
void c(f16mat3 m) {
}
void d(f16vec3 v) {
}
void e(float16_t f_1) {
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  f16vec3 v_6 = tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_7 = v_1.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3(v_4, v_6, tint_bitcast_to_f16(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
S v_8(uint start_byte_offset) {
  uvec4 v_9 = v_1.inner[(start_byte_offset / 16u)];
  int v_10 = int(v_9[((start_byte_offset & 15u) >> 2u)]);
  f16mat3 v_11 = v_2((8u + start_byte_offset));
  uvec4 v_12 = v_1.inner[((64u + start_byte_offset) / 16u)];
  return S(v_10, v_11, int(v_12[(((64u + start_byte_offset) & 15u) >> 2u)]));
}
S[4] v_13(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), 0), S(0, f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), 0));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a_2[v_15] = v_8((start_byte_offset + (v_15 * 128u)));
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_13(0u));
  b(v_8(256u));
  c(v_2(264u));
  d(tint_bitcast_to_f16(v_1.inner[1u].xy).xyz.zxy);
  e(tint_bitcast_to_f16(v_1.inner[1u].xy).xyz.zxy.x);
}
