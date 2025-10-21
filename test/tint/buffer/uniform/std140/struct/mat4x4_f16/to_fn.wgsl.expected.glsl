#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat4 m;
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
void c(f16mat4 m) {
}
void d(f16vec4 v) {
}
void e(float16_t f_1) {
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_6 = tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_7 = v_1.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_8 = tint_bitcast_to_f16(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_9 = v_1.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_4, v_6, v_8, tint_bitcast_to_f16(mix(v_9.xy, v_9.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
S v_10(uint start_byte_offset) {
  uvec4 v_11 = v_1.inner[(start_byte_offset / 16u)];
  int v_12 = int(v_11[((start_byte_offset % 16u) / 4u)]);
  f16mat4 v_13 = v_2((8u + start_byte_offset));
  uvec4 v_14 = v_1.inner[((64u + start_byte_offset) / 16u)];
  return S(v_12, v_13, int(v_14[(((64u + start_byte_offset) % 16u) / 4u)]));
}
S[4] v_15(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0));
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      a_2[v_17] = v_10((start_byte_offset + (v_17 * 128u)));
      {
        v_16 = (v_17 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(v_15(0u));
  b(v_10(256u));
  c(v_2(264u));
  d(tint_bitcast_to_f16(v_1.inner[1u].xy).ywxz);
  e(tint_bitcast_to_f16(v_1.inner[1u].xy).ywxz.x);
}
