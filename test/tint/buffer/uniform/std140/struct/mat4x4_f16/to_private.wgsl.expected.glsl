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
} v;
S p[4] = S[4](S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0));
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_16bit(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  f16vec4 v_6 = tint_bitcast_to_16bit(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  f16vec4 v_9 = tint_bitcast_to_16bit(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u))));
  uint v_10 = (24u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return f16mat4(v_3, v_6, v_9, tint_bitcast_to_16bit(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))));
}
S v_12(uint start_byte_offset) {
  uvec4 v_13 = v.inner[(start_byte_offset / 16u)];
  int v_14 = int(v_13[((start_byte_offset & 15u) >> 2u)]);
  f16mat4 v_15 = v_1((8u + start_byte_offset));
  uint v_16 = (64u + start_byte_offset);
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  return S(v_14, v_15, int(v_17[((v_16 & 15u) >> 2u)]));
}
S[4] v_18(uint start_byte_offset) {
  S a[4] = S[4](S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0), S(0, f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), 0));
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      a[v_20] = v_12((start_byte_offset + (v_20 * 128u)));
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_18(0u);
  p[1u] = v_12(256u);
  p[3u].m = v_1(264u);
  p[1u].m[0u] = tint_bitcast_to_16bit(v.inner[1u].xy).ywxz;
}
