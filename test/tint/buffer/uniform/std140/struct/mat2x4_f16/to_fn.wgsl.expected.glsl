#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  int before;
  f16mat2x4 m;
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
void c(f16mat2x4 m) {
}
void d(f16vec4 v) {
}
void e(float16_t f_1) {
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x4(v_4, tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
S v_6(uint start_byte_offset) {
  uvec4 v_7 = v_1.inner[(start_byte_offset / 16u)];
  int v_8 = int(v_7[((start_byte_offset % 16u) / 4u)]);
  f16mat2x4 v_9 = v_2((8u + start_byte_offset));
  uvec4 v_10 = v_1.inner[((64u + start_byte_offset) / 16u)];
  return S(v_8, v_9, int(v_10[(((64u + start_byte_offset) % 16u) / 4u)]));
}
S[4] v_11(uint start_byte_offset) {
  S a_2[4] = S[4](S(0, f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), 0));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a_2[v_13] = v_6((start_byte_offset + (v_13 * 128u)));
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
  a(v_11(0u));
  b(v_6(256u));
  c(v_2(264u));
  d(tint_bitcast_to_f16(v_1.inner[1u].xy).ywxz);
  e(tint_bitcast_to_f16(v_1.inner[1u].xy).ywxz.x);
}
